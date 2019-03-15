#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "mpool.h"
#include "mpool_memcheck.h"

#define MPOOL_CACHE_SIZE 10

#define NUM_POOLS (LG2_PAGE_SIZE - LG2_CACHELINE_SIZE + 1)
#if NUM_POOLS > 7
#error NUM_POOLS > 7
#endif


struct chunk_list {
	struct chunk_list * next;
};

struct mpool_cpu_cache {
	size_t elem_size;
	struct chunk_list * free;
	unsigned int num_free;
};

struct mpool {
	pthread_mutex_t lock;

	unsigned int num_free;
	struct chunk_list * free;

	size_t elem_size;
	size_t arena_size;
	uint8_t * arena CACHE_ALIGNED;
};

struct mpool_glob {
	struct mpool pools[NUM_POOLS];
};

static __thread struct mpool_cpu_cache pool_cache[NUM_POOLS] = {{0}};
static struct mpool_glob pool_glob = {0};


static void
mpool_init_arena(struct mpool * pool, size_t elem_size, size_t arena_size,
	int pool_index)
{
	void * tmp;
	uint8_t * ptr, *arena_end;

	assert(pool != NULL);

	pool->arena_size = arena_size;
	pool->elem_size = elem_size;
	pthread_mutex_init(&pool->lock, NULL);
	memset(pool->arena, 0, pool->arena_size);

	arena_end = ((uint8_t *) pool->arena) + pool->arena_size;
	arena_end -= (elem_size - 1);
	for (ptr = pool->arena; ptr < arena_end ; ptr += pool->elem_size) {
		tmp = pool->free;
		pool->free = VOIDPTR(ptr);
		pool->free->next = tmp;
		pool->num_free += 1;
	}

	MPOOL_CREATE_MEMPOOL(MPOOL_GET(pool_index), 0, 0);

	/* init pool cache */
	pool_cache[pool_index].elem_size = elem_size;
}


static void *
mpool_cache_align_ptr(void * ptr)
{
	uint8_t * _ptr;
	_ptr = (uint8_t *) (((uintptr_t) ptr) & ~((1ULL << CONFIG_LOG2_CPU_CACHELINE_SIZE) - 1ULL));

	if (ptr == _ptr)
		return ptr;
	else
		return _ptr + CACHELINE_SIZE;
}


NOINLINE int
mpool_create(void * arena, size_t total_size,
		unsigned int * weights, int weights_len)
{
	int i;
	uint8_t * arena_ptr;
	size_t elem_size, arena_size, total_weight;
	struct mpool * pool;

	assert(total_size >= CACHELINE_SIZE);
	assert(weights_len <= NUM_POOLS);

	total_weight = 0;
	for (i = 0 ; i < weights_len; i++)
		total_weight += (1 << i) * CACHELINE_SIZE * weights[i];
	assert(total_weight > 0);

	arena_ptr = mpool_cache_align_ptr(arena);
	total_size -= (size_t) (arena_ptr - (uint8_t *) arena);

	if (weights_len > NUM_POOLS || total_weight <= 0
			|| total_size < CACHELINE_SIZE)
		return -1;
	
	for (i = 0 ; i < weights_len ; i++) {
		if (weights[i] == 0)
			continue;

		elem_size = (1 << i) * CACHELINE_SIZE;
		arena_size = ((elem_size * total_size * weights[i]) / total_weight);
		pool = &pool_glob.pools[i];
		pool->arena = arena_ptr;
		arena_ptr += arena_size;

		mpool_init_arena(pool, elem_size, arena_size, i);
	}

	return 0;
}


NOINLINE
void mpool_destroy(void)
{
	int i;
	struct mpool * pool;

	for (i = 0 ; i < NUM_POOLS ; i++) {
		pool = &pool_glob.pools[i];
		if (pool->arena != NULL) {
			MPOOL_DESTROY_MEMPOOL(MPOOL_GET(i));
		}
	}
}


static ALWAYS_INLINE int
mpool_get_pool_index(size_t _size)
{
	uint64_t size = _size;

	if (size <= CACHELINE_SIZE)
		return 0;

	return 64 - LG2_CACHELINE_SIZE - __builtin_clzll(size - 1);
}


static int
mpool_fill_cache(struct mpool_cpu_cache *cache, struct mpool * pool)
{
	int i;
	void * ptr, * tmp;

	assert(pool != NULL);
	assert(cache != NULL);
	assert(cache->free == NULL);

	if (pthread_mutex_lock(&pool->lock) != 0)
		return -1;

	if (unlikely(pool->num_free < MPOOL_CACHE_SIZE)) {
		pthread_mutex_unlock(&pool->lock);
		return ENOMEM;
	}

	for (i = 0 ; i < MPOOL_CACHE_SIZE ; i++) {
		ptr = pool->free;
		pool->free = pool->free->next;

		tmp = cache->free;
		cache->free = ptr;
		cache->free->next = tmp;
	}

	pool->num_free -= MPOOL_CACHE_SIZE;
	cache->num_free += MPOOL_CACHE_SIZE;

	pthread_mutex_unlock(&pool->lock);
	return 0;
}


__attribute__((malloc))
__attribute__((alloc_size(1)))
void *
mpool_alloc(size_t size, int flags)
{
	void * ptr;
	int pool_index;
	struct mpool_cpu_cache * cache;

	(void) flags; /* for later user */

	pool_index = mpool_get_pool_index(size);
	if (unlikely(pool_index == NUM_POOLS))
		return NULL;
	cache = &pool_cache[pool_index];

	if (cache->num_free == 0) {
		if (unlikely(mpool_fill_cache(cache, &pool_glob.pools[pool_index]) != 0))
			return NULL;
		assert(cache->num_free != 0);
	}

	ptr = cache->free;
	cache->free = cache->free->next;
	cache->num_free -= 1;

	MPOOL_MEMPOOL_ALLOC(MPOOL_GET(pool_index), ptr, cache->elem_size);
	MPOOL_MAKE_MEM_UNDEFINED(ptr, size);
	MPOOL_MAKE_MEM_NOACCESS((uint8_t *) ptr + size, cache->elem_size - size);

	return ptr;
}


static void
mpool_empty_cache(struct mpool_cpu_cache *cache, struct mpool * pool)
{
	int i;
	void * ptr, * tmp;

	assert(pool != NULL);
	assert(cache != NULL);

	if (pthread_mutex_lock(&pool->lock) != 0)
		return;

	for (i = 0 ; i < MPOOL_CACHE_SIZE ; i++) {
		ptr = cache->free;
		cache->free = cache->free->next;

		tmp = pool->free;
		pool->free = ptr;
		pool->free->next = tmp;
	}

	cache->num_free -= MPOOL_CACHE_SIZE;
	pool->num_free += MPOOL_CACHE_SIZE;

	pthread_mutex_unlock(&pool->lock);
}


void mpool_free(void const * ptr, size_t size)
{
	int pool_index;
	struct mpool_cpu_cache * cache;
	struct chunk_list * tmp;

	if (ptr == NULL)
		return;

	pool_index = mpool_get_pool_index(size);
	cache = &pool_cache[pool_index];
	assert(pool_index < NUM_POOLS);

	MPOOL_MEMPOOL_FREE(MPOOL_GET(pool_index), ptr);
	MPOOL_MAKE_MEM_DEFINED(ptr, sizeof(uintptr_t));

	tmp = cache->free;
	cache->free = VOIDPTR(ptr);
	cache->free->next = tmp;
	cache->num_free += 1;

	if (cache->num_free > 2 * MPOOL_CACHE_SIZE)
		mpool_empty_cache(cache, &pool_glob.pools[pool_index]);

	return;
}


void * mpool_realloc(void const * ptr, size_t old_size, size_t new_size, int flags)
{
	void * tmp;

	assert(ptr != NULL || old_size == 0);
	if (unlikely(ptr == NULL && old_size != 0))
		return NULL;

	if (ptr != NULL
			&& mpool_get_pool_index(old_size) == mpool_get_pool_index(new_size)) {
		MPOOL_MAKE_MEM_UNDEFINED((const uint8_t *) ptr + old_size, new_size - old_size);
		return VOIDPTR(ptr);
	}

	tmp = mpool_alloc(new_size, flags);
	if (likely(tmp != NULL)) {
		if (ptr != NULL)
			memcpy(tmp, ptr, MIN(old_size, new_size));

		mpool_free(ptr, old_size);
	}
	return tmp;
}


NOINLINE void
mpool_stats(void)
{
	int i;
	size_t num_elem;
	struct mpool * pool;

	for (i = 0 ; i < NUM_POOLS ; i++) {
		pool = &pool_glob.pools[i];
		num_elem = pool->arena_size / pool->elem_size;
		printf("pool[%zd] %zd/%zd\n", pool->elem_size,
				num_elem - pool->num_free, num_elem);
	}
}

#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "check.h"
#include "common.h"
#include "mpool.h"

int
main(void)
{
	int rv;
	size_t i;
	void * ptr;
	void * arena;
	size_t arena_size;
	unsigned int weights[] = {1, 1, 1, 1, 1, 1, 1};

	arena_size = 1 << 20;
	arena = mmap(NULL, arena_size,
			PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_SHARED,
			-1, 0);
	check(arena != NULL);

	rv = mpool_create(arena, arena_size, weights, arraylen(weights));
	check(rv == 0);

	mpool_stats();
	printf("\n");

	/* smoke test */
	for (i = 1 ; i <= 4096 ; i<<=1) {
		ptr = mpool_alloc(i, 0);
		check(ptr != NULL);
		memset(ptr, 'a', i);
		mpool_free(ptr, i);
	}

	/* out of the pool */
	check(mpool_alloc(4097, 0) == NULL);

	/* realloc */
	ptr = mpool_realloc(NULL, 0, 0, 0);
	check(ptr != NULL);
	ptr = mpool_realloc(ptr, 0, 1, 0);
	check(ptr != NULL);
	ptr = mpool_realloc(ptr, 1, 2, 0);
	check(ptr != NULL);
	ptr = mpool_realloc(ptr, 2, 96, 0);
	check(ptr != NULL);
	mpool_free(ptr, 96);

	/* focus on single pool, to force cache refill */
	for (i = 0 ; i < 100 ; i++)
		check(mpool_alloc(42, 0) != NULL);

	/* should print 100 elems used in the 1st pool, 10 in all the others */
	mpool_stats();
	printf("\n");
		
	/* FIXME: don't free, destroy */
	mpool_destroy();
	munmap(arena, arena_size);

	return 0;
}

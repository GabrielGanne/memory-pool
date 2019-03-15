#define _GNU_SOURCE /* pthread_setaffinity_np() */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include "check.h"
#include "common.h"
#include "mpool.h"

#define NUM_THREADS 8
#define NUM_ALLOCS (PAGE_SIZE + 1)

static void *
mpool_test_thread(void * void_args)
{
	size_t i;
	void * ptr[NUM_ALLOCS];

	(void) void_args;

	for (i = 0 ; i < NUM_ALLOCS ; i++) {
		ptr[i] = mpool_alloc(i, 0);
		check(ptr[i] != NULL);
		memset(ptr[i], 'a', i);
		check(i <= PAGE_SIZE && ptr != NULL)
	}
	for (i = 0 ; i < NUM_ALLOCS ; i++)
		mpool_free(ptr[i], i);

	return NULL;
}

int
main(void)
{
	int rv, i;
	unsigned int weights[] = {1, 1, 1, 1, 1, 1, 1};
	cpu_set_t cpuset;
	void * arena;
	size_t arena_size;
	void * thread_rv[NUM_THREADS];
	pthread_t threads[NUM_THREADS] = {0};

	/* create mpool */
	arena_size = (1 << 24) * NUM_THREADS;
	arena = mmap(NULL, arena_size,
			PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_SHARED,
			-1, 0);
	check(arena != NULL);

	rv = mpool_create(arena, arena_size, weights, arraylen(weights));
	check(rv == 0);

	mpool_stats();
	printf("\n");

	CPU_ZERO(&cpuset);
	for (i = 0 ; i < NUM_THREADS ; i++) {
		rv = pthread_create(&threads[i], NULL, &mpool_test_thread, NULL);
		check(rv == 0);
		CPU_SET(i, &cpuset);
		rv = pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
		check(rv == 0);
	}

	/* join all threads */
	for (i = 0 ; i < NUM_THREADS ; i++) {
		rv = pthread_join(threads[i], (void **) &thread_rv[i]);
		check(rv == 0);
		check(thread_rv[i] == NULL);
	}
			
	/* destroy mpool */
	mpool_destroy();
	munmap(arena, arena_size);
	return 0;
}

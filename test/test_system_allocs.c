#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"

#define NUM_PTRS (1 << 10)
#define NUM_ALLOCS 1000
#define MAX_SIZE (1 << 14) /* 16KB */

static inline size_t random_size(void)
{
	return rand() % MAX_SIZE;
}

static inline int random_ptr(void)
{
	return rand() % NUM_PTRS;
}


int
main(int argc, char ** argv)
{
	int i;
	int size;
	int ptr_index;
	void * ptr;
	void * ptr_array[NUM_PTRS] = {0};

	if (argc == 2)
		srand(atoi(argv[1]));

	for (i = 0 ; i < NUM_ALLOCS ; i++) {
		ptr_index = random_ptr();
		size = random_size();
		ptr = ptr_array[ptr_index];

		if ((size % 17) == 0) {
			free(ptr);
			ptr = malloc(size);
		} else {
			ptr = realloc(ptr, size);
		}
		check(ptr != NULL);
		memset(ptr, 'x', size);
		ptr_array[ptr_index] = ptr;
	}

	for (i = 0 ; i < NUM_PTRS ; i++)
		free(ptr_array[i]);

	return 0;
}

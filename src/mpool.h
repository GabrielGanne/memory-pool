#ifndef MPOOL_H
#define MPOOL_H

#include <stdlib.h>

int mpool_create(void * arena, size_t total_size, unsigned int * weights, int weights_len);
void mpool_destroy(void);

void * mpool_alloc(size_t size, int flags);
void mpool_free(void const * ptr, size_t size);
void * mpool_realloc(void const * ptr, size_t old_size, size_t new_size, int flags);

void mpool_stats(void);

#endif /* MPOOL_H */

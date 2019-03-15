#ifndef MPOOL_MEMCHECK_H
#define MPOOL_MEMCHECK_H

/* this indirection is here so that the library can be compiled without
 * memcheck installed */
#ifdef MEMCHECK
/* This will trigger quite some compiler warnings (mostly reserved-id-macro).
 * Let's just not care about it */
#include <valgrind/memcheck.h>

#define MPOOL_GET(index) (&pool_glob.pools[(index)].arena)
#define MPOOL_CREATE_MEMPOOL VALGRIND_CREATE_MEMPOOL
#define MPOOL_DESTROY_MEMPOOL VALGRIND_DESTROY_MEMPOOL
#define MPOOL_MEMPOOL_ALLOC VALGRIND_MEMPOOL_ALLOC
#define MPOOL_MEMPOOL_FREE VALGRIND_MEMPOOL_FREE

#define MPOOL_MAKE_MEM_NOACCESS VALGRIND_MAKE_MEM_NOACCESS
#define MPOOL_MAKE_MEM_UNDEFINED VALGRIND_MAKE_MEM_UNDEFINED
#define MPOOL_MAKE_MEM_DEFINED VALGRIND_MAKE_MEM_DEFINED

#else /* MEMCHECK */

#define MPOOL_CREATE_MEMPOOL(...)
#define MPOOL_DESTROY_MEMPOOL(...)
#define MPOOL_MEMPOOL_ALLOC(...)
#define MPOOL_MEMPOOL_FREE(...)

#define MPOOL_MAKE_MEM_NOACCESS(...)
#define MPOOL_MAKE_MEM_UNDEFINED(...)
#define MPOOL_MAKE_MEM_DEFINED(...)

#endif /* MEMCHECK */

#endif /* MPOOL_MEMCHECK_H */

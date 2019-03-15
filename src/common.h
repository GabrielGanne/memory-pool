#ifndef COMMON_H
#define COMMON_H

/* config values */
#define LG2_CACHELINE_SIZE CONFIG_LOG2_CPU_CACHELINE_SIZE
#define CACHELINE_SIZE (1 << LG2_CACHELINE_SIZE)

#define LG2_PAGE_SIZE CONFIG_LOG2_CPU_PAGE_SIZE
#define PAGE_SIZE (1 << LG2_PAGE_SIZE)

#define NR_CPUS CONFIG_NR_CPUS

/* common macros */

#define arraylen(x) (sizeof(x) / sizeof(*(x)))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define likely(expr)   __builtin_expect(!!(expr), 1)
#define unlikely(expr) __builtin_expect(!!(expr), 0)

#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))
#define PACKED __attribute__((packed))
#define CACHE_ALIGNED __attribute__ ((aligned (CACHELINE_SIZE)))

/* silence warnings about void const */
#define VOIDPTR(ptr) \
	(void *)(uintptr_t)(ptr)

#endif /* COMMON_H */

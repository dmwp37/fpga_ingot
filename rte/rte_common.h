#ifndef _RET_COMMON_H_
#define _RET_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*================================================================================================*/
/*================================================================================================*/
#ifndef __ELASTERROR                                                                                                                                                                         
/**
 * Check if we have a defined value for the max system-defined errno values.
 * if no max defined, start from 1000 to prevent overlap with standard values
 */
#define __ELASTERROR 1000
#endif

/** Error types */
enum {
    RTE_MIN_ERRNO = __ELASTERROR, /**< Start numbering above std errno vals */

    E_RTE_SECONDARY, /**< Operation not allowed in secondary processes */
    E_RTE_NO_CONFIG, /**< Missing rte_config */
    E_RTE_NO_TAILQ,  /**< Uninitialised TAILQ */

    RTE_MAX_ERRNO    /**< Max RTE error number */
};

#define RTE_RING_SPLIT_PROD_CONS

#define CACHE_LINE_SIZE 64
#define CACHE_LINE_MASK (CACHE_LINE_SIZE-1)
#define __rte_cache_aligned __attribute__((__aligned__(CACHE_LINE_SIZE)))
#define likely(x)  __builtin_expect((x),1)
#define unlikely(x)  __builtin_expect((x),0)
    
/**
 * Compiler barrier.
 *
 * Guarantees that operation reordering does not occur at compile time 
 * for operations directly before and after the barrier.
 */
#define rte_compiler_barrier() do {     \
    asm volatile ("" : : : "memory");   \
} while(0)

#include <emmintrin.h>
/**
 * PAUSE instruction for tight loops (avoid busy waiting)
 */
static inline void
rte_pause (void)                                                                                                                                                                             
{
    _mm_pause();
}
/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no
 * bigger than the first parameter. Second parameter must be a
 * power-of-two value.
 */
#define RTE_ALIGN_FLOOR(val, align) \
    (typeof(val))((val) & (~((typeof(val))((align) - 1))))

/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no lower
 * than the first parameter. Second parameter must be a power-of-two
 * value.
 */
#define RTE_ALIGN_CEIL(val, align) \
    RTE_ALIGN_FLOOR(((val) + ((typeof(val)) (align) - 1)), align)

/**
 * Macro to align a value to a given power-of-two. The resultant
 * value will be of the same type as the first parameter, and
 * will be no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 * This function is the same as RTE_ALIGN_CEIL
 */
#define RTE_ALIGN(val, align) RTE_ALIGN_CEIL(val, align)

extern int RTE_BUILD_BUG_ON_detected_error;
#define RTE_BUILD_BUG_ON(condition) do {             \
    ((void)sizeof(char[1 - 2*!!(condition)]));   \
    if (condition)                               \
        RTE_BUILD_BUG_ON_detected_error = 1; \
} while(0)

/*================================================================================================*/
/*================================================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RET_COMMON_H_ */

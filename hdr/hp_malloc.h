#ifndef _HP_MALLOC_H
#define _HP_MALLOC_H
/*==================================================================================================

    Module Name:  hp_malloc.h

    General Description: This file provides huge page malloc interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                              MACROS
==================================================================================================*/
#define HUGE_PASE_SIZE 0x200000  /*2M, get from /sys/kernel/mm/hugepages or /proc/meminfo*/

/*==================================================================================================
                                               ENUMS
==================================================================================================*/

/*==================================================================================================
                                   STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
typedef uint64_t phys_addr_t;

typedef struct
{
    void*       base;
    size_t      size;
    phys_addr_t phys_addr;
} hp_t;

/*==================================================================================================
                                   GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/
hp_t* hp_alloc(size_t size);
void  hp_free(hp_t* p_hp);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _HP_MALLOC_H  */


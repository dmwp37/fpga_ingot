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
hp_t* malloc_huge_pages(size_t size);
void  free_huge_pages(hp_t* p_hp);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _HP_MALLOC_H  */


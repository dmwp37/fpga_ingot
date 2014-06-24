#ifndef _FPGA_UIO_H
#define _FPGA_UIO_H
/*==================================================================================================

    Module Name:  fpga_uio.h

    General Description: This file provides driver interface for FPGA uio

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>

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

/*==================================================================================================
                                   GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/
bool  FPGA_UIO_init();
void  FPGA_UIO_exit();
void* FPGA_UIO_get_base();

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _FPGA_UIO_H  */


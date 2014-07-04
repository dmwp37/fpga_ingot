#ifndef _FPGA_RX_H_
#define _FPGA_RX_H_
/*==================================================================================================

    Module Name:  fpga_rx.h

    General Description: This file provides driver interface for FPGA RX

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/

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
void fpga_rx_init();
void fpga_rx_exit();

int fpga_rx(int port, void* buf, size_t len);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _FPGA_RX_H_  */


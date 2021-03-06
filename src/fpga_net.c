/*==================================================================================================

    Module Name:  fpga_net.c

    General Description: Implements the FPGA net driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include "mem_map.h"
#include "rx_mbuf.h"
#include "fpga_drv.h"
#include "fpga_rx.h"
#include "fpga_tx.h"
#include "fpga_net.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static void fpga_net_exit();

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init fpga net driver.

@return 0 if success
*//*==============================================================================================*/
int fpga_net_init()
{
    return atexit(fpga_net_exit) ||
           mem_map_init() ||
           rx_mbuf_init() ||
           fpga_drv_init() ||
           fpga_rx_init() ||
           fpga_tx_init();
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief fpga net driver clean up

@return 0 if success
*//*==============================================================================================*/
void fpga_net_exit()
{
    fpga_rx_exit();
    fpga_drv_exit();
    rx_mbuf_exit();
    mem_map_exit();
}


/*==================================================================================================

    Module Name:  fpga_drv.c

    General Description: Implements the FPGA ingot driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "jspec/mmap.h"
#include "jspec/ingot.h"
#include "jspec/fabric_defines.h"
#include "fpga_uio.h"
#include "fpga_drv.h"

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
@brief Reset the FPGA

*//*==============================================================================================*/
void FPGA_DRV_reset(void)
{
    ingot_t* ingot_reg = (ingot_t*)FPGA_UIO_get_base();

    ingot_fabric_write(FAB_NID_QMGR, QMGR_CSR_STATS_CLR, 0);
    ingot_reg->warm_reset = 0xbaad;

    printf("reset FPGA\n");
    usleep(10000);
}

/*=============================================================================================*//**
@brief Get the FPGA version

*//*==============================================================================================*/
uint64_t FPGA_DRV_get_version()
{
    ingot_t* ingot_reg = (ingot_t*)FPGA_UIO_get_base();

    return ingot_reg->version;
}

uint64_t ingot_fabric_read(uint32_t nid, uint32_t cntl, uint32_t upper_32_bits)
{
    ingot_t* ingot_reg = (ingot_t*)FPGA_UIO_get_base();
    uint32_t indirect_reg;
    uint64_t indirect_read;
    uint64_t results = 0;

    indirect_reg   = INGOT_REG(nid, cntl);
    indirect_read  = ((uint64_t)indirect_reg << 32);
    indirect_read |= ((uint64_t)upper_32_bits);

    /* here should prevent optimization */
    ingot_reg->fab_read = indirect_read;

    results = ingot_reg->fab_read;
    results = ingot_reg->fab_read;
    results = ingot_reg->fab_read;
    results = ingot_reg->fab_read;
    results = ingot_reg->fab_read;

    results = results & 0x00000000FFFFFFFF;
    return results;
}

void ingot_fabric_write(uint32_t nid, uint32_t cntl, uint32_t data)
{
    ingot_t* ingot_reg = (ingot_t*)FPGA_UIO_get_base();
    uint32_t indirect_reg;
    uint64_t indirect_write;

    indirect_reg    = INGOT_REG(nid, cntl);
    indirect_write  = ((uint64_t)indirect_reg << 32);
    indirect_write |= data;

    ingot_reg->fab_write = indirect_write;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/


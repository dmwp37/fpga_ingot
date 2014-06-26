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
#include <sys/queue.h>
#include "uio.h"
#include "fpga_drv.h"
#include "jspec/mmap.h"
#include "jspec/ingot.h"
#include "jspec/fabric_defines.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/
#define FPGA_INGOT_DRIVER "Ingot FPGA UIO"

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
uint64_t ingot_fabric_read(uint32_t nid, uint32_t cntl, uint32_t upper_32_bits);
void     ingot_fabric_write(uint32_t nid, uint32_t cntl, uint32_t data);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static uio_t*   fpga_drv_uio = NULL;
static ingot_t* ingot_reg    = NULL;

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init the FPGA driver

@return -1 if error happened, 0 for success
*//*==============================================================================================*/
int fpga_drv_init()
{
    fpga_drv_uio = uio_init(FPGA_INGOT_DRIVER);

    if (fpga_drv_uio != NULL)
    {
        ingot_reg = (ingot_t*)fpga_drv_uio->base;
        return 0;
    }

    return -1;
}

/*=============================================================================================*//**
@brief release the FPGA driver

*//*==============================================================================================*/
void fpga_drv_exit()
{
    uio_exit(fpga_drv_uio);
    fpga_drv_uio = NULL;
    ingot_reg    = NULL;
}

/*=============================================================================================*//**
@brief Reset the FPGA

*//*==============================================================================================*/
void fpga_drv_reset(void)
{
    ingot_fabric_write(FAB_NID_QMGR, QMGR_CSR_STATS_CLR, 0);
    ingot_reg->warm_reset = 0xbaad;

    printf("reset FPGA\n");
    usleep(10000);
}

/*=============================================================================================*//**
@brief Get the FPGA version

*//*==============================================================================================*/
uint64_t fpga_drv_get_version()
{
    return ingot_reg->version;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/
uint64_t ingot_fabric_read(uint32_t nid, uint32_t cntl, uint32_t upper_32_bits)
{
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
    uint32_t indirect_reg;
    uint64_t indirect_write;

    indirect_reg    = INGOT_REG(nid, cntl);
    indirect_write  = ((uint64_t)indirect_reg << 32);
    indirect_write |= data;

    ingot_reg->fab_write = indirect_write;
}


#ifndef _INCLUDE_MMAP_H
#define _INCLUDE_MMAP_H

// include the fabric node IDs
#include "fabric_defines.h"

// All fabric nodes must use port 0 for the control registers
#define MAP_CTLREG_PORT       0

// Bit offsets within the address
#define MAP_ADDR_NID_OFFSET  28
#define MAP_ADDR_PORT_OFFSET 24

// Generate internal memory addresses
#define INGOT_ADDR(nid, port, address)  \
    ( ((nid) << MAP_ADDR_NID_OFFSET) | ((port) << MAP_ADDR_PORT_OFFSET) | (address) )

#define INGOT_REG(nid, register) INGOT_ADDR( (nid), (MAP_CTLREG_PORT), (register) )


// All control register modules must have the ID register at address 0.
#define CTLREG_ID              0x000

// DMA Control Registers
#define CTLREG_DMA_SRC_ADDR    0x004
#define CTLREG_DMA_DEST_ADDR   0x008
#define CTLREG_DMA_LENGTH      0x00c
#define CTLREG_DMA_STATUS      0x010

// DDR Control Registers
#define CTLREG_DDR_CTL         0x004
#define CTLREG_DDR_STATUS      0x008

#define CTLREG_WR_RAM_ADDR     0x010
#define CTLREG_WR_LCL_ADDR     0x014
#define CTLREG_WR_STATUS       0x018

#define CTLREG_RD_RAM_ADDR     0x020
#define CTLREG_RD_LCL_ADDR     0x024
#define CTLREG_RD_STATUS       0x028

#endif

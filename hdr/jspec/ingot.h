/*
 * $Id: struct_h.c,v 1.41 2013/10/15 17:02:24 cleung Exp $
 *
 * Copyright (c) 2014, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

/*
 *
 * @file ingot.h
 * @brief ASIC Specification for ingot chip
 *
 * Generated by:
 * jhansell on Fri Mar 28 11:27:46 2014 using JSPEC 1.15.21
 * Do not edit!
 *
 */
#ifndef __INGOT_H__
#define __INGOT_H__

#pragma pack(1)

typedef volatile uint64_t ingot_regid;
    #define   INGOT_REGID_RESERVED_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 8) & 0xffffffffffffff00ULL) | ((CUR) & ~0xffffffffffffff00ULL))
    #define   INGOT_REGID_RESERVED_GET(CUR) (((CUR) >> 8) & 0x00ffffffffffffffULL)
    #define   INGOT_REGID_RESERVED_MSB 63
    #define   INGOT_REGID_RESERVED_LSB 8
    #define   INGOT_REGID_RESERVED_FWIDTH 56
    #define   INGOT_REGID_ID_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x00000000000000ffULL) | ((CUR) & ~0x00000000000000ffULL))
    #define   INGOT_REGID_ID_GET(CUR) ((CUR) & 0x00000000000000ffULL)
        #define   INGOT_REGID_ID_INGOT 0x1
        #define   INGOT_REGID_ID_DEVBOARD 0x2
        #define   INGOT_REGID_ID_KIELBASA 0x3
        #define   INGOT_REGID_ID_BRATWURST 0x4
    #define   INGOT_REGID_ID_MSB 7
    #define   INGOT_REGID_ID_LSB 0
    #define   INGOT_REGID_ID_FWIDTH 8

typedef volatile uint64_t ingot_regvn;
    #define   INGOT_REGVN_BUILD_ID_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 32) & 0xffffffff00000000ULL) | ((CUR) & ~0xffffffff00000000ULL))
    #define   INGOT_REGVN_BUILD_ID_GET(CUR) (((CUR) >> 32) & 0x00000000ffffffffULL)
    #define   INGOT_REGVN_BUILD_ID_MSB 63
    #define   INGOT_REGVN_BUILD_ID_LSB 32
    #define   INGOT_REGVN_BUILD_ID_FWIDTH 32
    #define   INGOT_REGVN_MAJOR_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 16) & 0x00000000ffff0000ULL) | ((CUR) & ~0x00000000ffff0000ULL))
    #define   INGOT_REGVN_MAJOR_GET(CUR) (((CUR) >> 16) & 0x000000000000ffffULL)
    #define   INGOT_REGVN_MAJOR_MSB 31
    #define   INGOT_REGVN_MAJOR_LSB 16
    #define   INGOT_REGVN_MAJOR_FWIDTH 16
    #define   INGOT_REGVN_MINOR_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x000000000000ffffULL) | ((CUR) & ~0x000000000000ffffULL))
    #define   INGOT_REGVN_MINOR_GET(CUR) ((CUR) & 0x000000000000ffffULL)
    #define   INGOT_REGVN_MINOR_MSB 15
    #define   INGOT_REGVN_MINOR_LSB 0
    #define   INGOT_REGVN_MINOR_FWIDTH 16

typedef volatile uint64_t ingot_reg64;
    #define   INGOT_REG64_VALUE_SET(CUR, NEWVAL) ((uint64_t)(NEWVAL))
    #define   INGOT_REG64_VALUE_GET(CUR) (CUR)
    #define   INGOT_REG64_VALUE_MSB 63
    #define   INGOT_REG64_VALUE_LSB 0
    #define   INGOT_REG64_VALUE_FWIDTH 64

typedef volatile uint64_t ingot_regLA;
    #define   INGOT_REGLA_LENGTH_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 32) & 0xffffffff00000000ULL) | ((CUR) & ~0xffffffff00000000ULL))
    #define   INGOT_REGLA_LENGTH_GET(CUR) (((CUR) >> 32) & 0x00000000ffffffffULL)
    #define   INGOT_REGLA_LENGTH_MSB 63
    #define   INGOT_REGLA_LENGTH_LSB 32
    #define   INGOT_REGLA_LENGTH_FWIDTH 32
    #define   INGOT_REGLA_ADDRESS_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x00000000ffffffffULL) | ((CUR) & ~0x00000000ffffffffULL))
    #define   INGOT_REGLA_ADDRESS_GET(CUR) ((CUR) & 0x00000000ffffffffULL)
    #define   INGOT_REGLA_ADDRESS_MSB 31
    #define   INGOT_REGLA_ADDRESS_LSB 0
    #define   INGOT_REGLA_ADDRESS_FWIDTH 32
typedef volatile uint64_t ingot_regQL;
    #define   INGOT_REGQL_COMPLETION_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 32) & 0xffffffff00000000ULL) | ((CUR) & ~0xffffffff00000000ULL))
    #define   INGOT_REGQL_COMPLETION_GET(CUR) (((CUR) >> 32) & 0x00000000ffffffffULL)
    #define   INGOT_REGQL_COMPLETION_MSB 63
    #define   INGOT_REGQL_COMPLETION_LSB 32
    #define   INGOT_REGQL_COMPLETION_FWIDTH 32
    #define   INGOT_REGQL_REQUEST_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x00000000ffffffffULL) | ((CUR) & ~0x00000000ffffffffULL))
    #define   INGOT_REGQL_REQUEST_GET(CUR) ((CUR) & 0x00000000ffffffffULL)
    #define   INGOT_REGQL_REQUEST_MSB 31
    #define   INGOT_REGQL_REQUEST_LSB 0
    #define   INGOT_REGQL_REQUEST_FWIDTH 32

typedef volatile uint64_t ingot_regWR;
    #define   INGOT_REGWR_RESET_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x000000000000ffffULL) | ((CUR) & ~0x000000000000ffffULL))
    #define   INGOT_REGWR_RESET_GET(CUR) ((CUR) & 0x000000000000ffffULL)
        #define   INGOT_REGWR_RESET_IGNORE 0x0
        #define   INGOT_REGWR_RESET_CLEAR 0xbaad
    #define   INGOT_REGWR_RESET_MSB 15
    #define   INGOT_REGWR_RESET_LSB 0
    #define   INGOT_REGWR_RESET_FWIDTH 16

typedef volatile uint64_t ingot_regAD;
    #define   INGOT_REGAD_ADDRESS_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 32) & 0xffffffff00000000ULL) | ((CUR) & ~0xffffffff00000000ULL))
    #define   INGOT_REGAD_ADDRESS_GET(CUR) (((CUR) >> 32) & 0x00000000ffffffffULL)
    #define   INGOT_REGAD_ADDRESS_MSB 63
    #define   INGOT_REGAD_ADDRESS_LSB 32
    #define   INGOT_REGAD_ADDRESS_FWIDTH 32
    #define   INGOT_REGAD_DATA_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x00000000ffffffffULL) | ((CUR) & ~0x00000000ffffffffULL))
    #define   INGOT_REGAD_DATA_GET(CUR) ((CUR) & 0x00000000ffffffffULL)
    #define   INGOT_REGAD_DATA_MSB 31
    #define   INGOT_REGAD_DATA_LSB 0
    #define   INGOT_REGAD_DATA_FWIDTH 32
typedef volatile uint64_t ingot_regLB;
    #define   INGOT_REGLB_LENGTH_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 48) & 0xffff000000000000ULL) | ((CUR) & ~0xffff000000000000ULL))
    #define   INGOT_REGLB_LENGTH_GET(CUR) (((CUR) >> 48) & 0x000000000000ffffULL)
    #define   INGOT_REGLB_LENGTH_MSB 63
    #define   INGOT_REGLB_LENGTH_LSB 48
    #define   INGOT_REGLB_LENGTH_FWIDTH 16
    #define   INGOT_REGLB_QUEUE_SET(CUR, NEWVAL) (((((uint64_t)(NEWVAL)) << 40) & 0x0000ff0000000000ULL) | ((CUR) & ~0x0000ff0000000000ULL))
    #define   INGOT_REGLB_QUEUE_GET(CUR) (((CUR) >> 40) & 0x00000000000000ffULL)
    #define   INGOT_REGLB_QUEUE_MSB 47
    #define   INGOT_REGLB_QUEUE_LSB 40
    #define   INGOT_REGLB_QUEUE_FWIDTH 8
    #define   INGOT_REGLB_BUF_NUM_SET(CUR, NEWVAL) ((((uint64_t)(NEWVAL)) & 0x00000000000000ffULL) | ((CUR) & ~0x00000000000000ffULL))
    #define   INGOT_REGLB_BUF_NUM_GET(CUR) ((CUR) & 0x00000000000000ffULL)
    #define   INGOT_REGLB_BUF_NUM_MSB 7
    #define   INGOT_REGLB_BUF_NUM_LSB 0
    #define   INGOT_REGLB_BUF_NUM_FWIDTH 8

typedef struct ingot_s
{
    ingot_regid   device_id;              /* 0x0   */
    ingot_regvn   version;                /* 0x8   */
    ingot_reg64   revision_tag1;          /* 0x10  */
    ingot_reg64   revision_tag2;          /* 0x18  */
    ingot_reg64   dma_rd_host_addr;       /* 0x20  */
    ingot_regLA   dma_rd_local;           /* 0x28  */
    ingot_regQL   dma_rd_level;           /* 0x30  */
    ingot_reg64   dma_rd_loopcount;       /* 0x38  */
    ingot_reg64   dma_wr_host_addr;       /* 0x40  */
    ingot_regLA   dma_wr_local;           /* 0x48  */
    ingot_regQL   dma_wr_level;           /* 0x50  */
    ingot_reg64   dma_wr_loopcount;       /* 0x58  */
    ingot_regLA   dma_rw_start;           /* 0x60  */
    unsigned char FILLER_0x68[0x8];
    ingot_reg64   dma_timer;              /* 0x70  */
    ingot_reg64   dma_gapcnt;             /* 0x78  */
    ingot_reg64   led;                    /* 0x80  */
    ingot_regWR   warm_reset;             /* 0x88  */
    unsigned char FILLER_0x88[0x70];
    ingot_reg64   dma_buf_offset;         /* 0x100 */
    unsigned char FILLER_0x108[0xf8];
    ingot_regAD   fab_write;              /* 0x200 */
    ingot_regAD   fab_read;               /* 0x208 */
    unsigned char FILLER_0x210[0xf0];
    ingot_reg64   rx_buf_base;            /* 0x300 */
    ingot_reg64   rx_desc_base;           /* 0x308 */
    ingot_reg64   tx_buf_base;            /* 0x310 */
    ingot_reg64   tx_desc_base;           /* 0x318 */
    ingot_regLB   tx_packet;              /* 0x320 */
    ingot_regLB   rx_packet;              /* 0x328 */
    unsigned char FILLER_0x330[0xcd0];
} ingot_t;

#pragma pack()
#endif /*__INGOT_H__*/


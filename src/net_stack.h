#ifndef _NET_STACK_H
#define _NET_STACK_H
/*==================================================================================================

    Module Name:  net_stack.h

    General Description: This file provides packet header definition

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
typedef enum
{
    BCM_GE_0  = 1,
    BCM_GE_1  = 2,
    BCM_GE_2  = 3,
    BCM_GE_3  = 4,
    BCM_GE_4  = 5,
    BCM_GE_5  = 6,
    BCM_GE_6  = 7,
    BCM_GE_7  = 8,
    BCM_GE_8  = 9,
    BCM_GE_9  = 10,
    BCM_GE_10 = 11,
    BCM_GE_11 = 12,
    BCM_GE_12 = 13,
    BCM_GE_13 = 14,
    BCM_GE_14 = 15,
    BCM_GE_15 = 16,
    BCM_WTB_1 = 17,
    BCM_WTB_2 = 25,
    BCM_XE_0  = 50,
    BCM_XE_1  = 51,
    BCM_XE_2  = 52,
    BCM_XE_3  = 53,
} bcm_port_t;

/*==================================================================================================
                                   STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
typedef struct
{
    volatile uint64_t* bufptr;
    volatile uint64_t  buflen;
} tx_descp_entry_t;

typedef struct
{
    volatile uint64_t  buflen;
    volatile uint64_t* bufptr;
} rx_descp_entry_t;

typedef union
{
    struct
    {
        uint8_t transmit_queue;
        uint8_t rsvd_1[15];
    } tx;

    struct
    {
        uint32_t buf_len;
        uint32_t reserved;
        uint64_t rx_index;
    } rx;
} meta_header_t;


typedef struct
{
    /* "Legacy" PPD Overlay 1 */     /* Byte # */
    uint32_t start              : 8; /* 0 */
    uint32_t tc                 : 4; /* 1 */
    uint32_t mcst               : 1;
    uint32_t _rsvd1             : 3;
    uint32_t dst_mod            : 8; /* 2 */
    uint32_t dst_port           : 8; /* 3 */
    uint32_t src_mod            : 8; /* 4 */
    uint32_t src_port           : 8; /* 5 */
    uint32_t lbid               : 8; /* 6 */
    uint32_t ppd_type           : 3; /* 7 */
    uint32_t _rsvd2             : 2;
    uint32_t ehv                : 1;
    uint32_t dp                 : 2;
    uint32_t mirror             : 1; /* 8 */
    uint32_t mirror_done        : 1;
    uint32_t mirror_only        : 1;
    uint32_t ingress_tagged     : 1;
    uint32_t lag_failover       : 1;
    uint32_t donot_learn        : 1;
    uint32_t donot_modify       : 1;
    uint32_t dst_t              : 1;
    uint32_t vc_label_19_16     : 4; /* 9 */
    uint32_t label_present      : 1;
    uint32_t l3                 : 1;
    uint32_t label_overlay_type : 2;
    uint32_t vc_label_15_8      : 8; /* 10 */
    uint32_t vc_label_7_0       : 8; /* 11 */
    uint32_t vlan_id_hi         : 4; /* 12 */
    uint32_t vlan_cfi           : 1;
    uint32_t vlan_pri           : 3;
    uint32_t vlan_id_lo         : 8; /* 13 */
    uint32_t opcode             : 3; /* 14 */
    uint32_t preserve_dot1p     : 1;
    uint32_t preserve_dscp      : 1;
    uint32_t src_t              : 1;
    uint32_t pfm                : 2;
    uint32_t _rsvd5             : 5; /* 15 */
    uint32_t hdr_ext_len        : 3;
} higig2_header_t;

typedef struct
{
    meta_header_t   meta;
    higig2_header_t hg2;
    uint8_t         buf[0]; /* Remaining 2k-32Bytes; */
} packet_buf_t;

/*==================================================================================================
                                   GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _NET_STACK_H  */


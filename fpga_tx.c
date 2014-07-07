/*==================================================================================================

    Module Name:  fpga_tx.c

    General Description: Implements the FPGA tx driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <string.h>
#include <errno.h>
#include "rte_common.h"
#include "mem_map.h"
#include "fpga_drv.h"
#include "fpga_net.h"
#include "fpga_tx.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/
#define TX_RING_MASK (TX_DESCRIPTOR_COUNT - 1)

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static inline void setup_packet(packet_buf_t* packet, int port, const void* buf, size_t len);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static int tx_global_queue = TX_QUEUE_FPGA_LOOP;

static uint64_t tx_packet_num  = 0;
static uint32_t tx_dropped_num = 0;
/* static uint32_t tx_error_num = 0; */

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init the FPGA tx driver

@return 0 if success
*//*==============================================================================================*/
int fpga_tx_init()
{
    phys_addr_t phys_base = global_mem->phys_addr;

    ingot_reg->tx_desc_base = phys_base + TX_DESCRIPTOR_OFFSET;
    ingot_reg->tx_buf_base  = phys_base + TX_MBUF_OFFSET;

    memset(global_mem->base + TX_DESCRIPTOR_OFFSET, 0, TX_DESCRIPTOR_SIZE);

    return 0;
}

/*=============================================================================================*//**
@brief config fpga net driver

@param[in] queue - which FPGA tx queue to use
                                                                                                  *
@return 0 if success
*//*==============================================================================================*/
int fpga_net_config(tx_queue_t queue)
{
    if (queue > TX_QUEUE_MAX)
    {
        printf("%s(): unsupported tx queue: %d", __func__, queue);
        return -1;
    }

    tx_global_queue = queue;
    return 0;
}

/*=============================================================================================*//**
@brief transmit a packet over a port.

@param[in] port - which port to send packet
@param[in] buf  - the buffer contains the packet
@param[in] len  - packet buffer length

@return 0 if success
*//*==============================================================================================*/
int fpga_net_tx(fpga_net_port_t port, const void* buf, size_t len)
{
    static volatile uint32_t tx_head = 0;

    uint32_t          head;
    uint32_t          idx;
    uint32_t          retry     = 0;
    tx_descp_entry_t* p_tx_desc = global_mem->base + TX_DESCRIPTOR_OFFSET;
    uint64_t          reg       = 0;
    int               success;

    if (port >= FPGA_PORT_MAX)
    {
        printf("TX specify an invalid port number: #%d\n", port);
        return -ENOTSUP;
    }

    tx_packet_num++;

    do
    {
        head = tx_head;
        idx  = head & TX_RING_MASK;

        /* check that we have tx entry available in ring */
        if (unlikely(p_tx_desc[idx].bufptr != NULL))
        {
            if (retry > 10000000)
            {
                printf("TX stuck while processing TX Descp #%d\n", idx);
                tx_dropped_num++;
                return -ENOBUFS;
            }
            retry++;
            continue;
        }
        /* test and increase tx_head atomically */
        success = rte_atomic32_cmpset(&tx_head, head, head + 1);
    } while (unlikely(success == 0));

    /* prepare mbuf data to tx */
    packet_buf_t* packet = (packet_buf_t*)(global_mem->base + TX_MBUF_OFFSET + MBUF_SIZE * idx);
    setup_packet(packet, port, buf, len);
    p_tx_desc[idx].bufptr = (uint64_t*)packet;
    /* add the header length to packet */
    len += sizeof(packet_buf_t);

    /* prepare the register */
    reg  = INGOT_REGLB_LENGTH_SET(reg, len);
    reg  = INGOT_REGLB_QUEUE_SET(reg, tx_global_queue);
    reg |= idx;

    rte_compiler_barrier();

    /* write to the fpga hardware */
    ingot_reg->tx_packet = reg;

    return 0;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

void setup_packet(packet_buf_t* packet, int port, const void* buf, size_t len)
{
    /* init the header */
    memset(packet, 0, sizeof(packet_buf_t));

    /* meta header */
    packet->meta.tx.transmit_queue = tx_global_queue;

    /* higig2 header */
    packet->hg2.start      = 0xfb;
    packet->hg2.tc         = 0x01;
    packet->hg2.src_mod    = 0x01;
    packet->hg2.dst_mod    = 0x00;
    packet->hg2.dst_port   = port;
    packet->hg2.src_port   = (tx_global_queue != TX_QUEUE_FPGA_LOOP) ? 0 : port;
    packet->hg2.lbid       = 0x09;
    packet->hg2.vlan_id_lo = 0x01;
    packet->hg2.opcode     = 0x01;

    /* copy rest data to the tx mbuf */
    memcpy(packet->buf, buf, len);
}


/*==================================================================================================

    Module Name:  fpga_tx.c

    General Description: Implements the FPGA tx driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <sched.h>
#include "dg_dbg.h"
#include "rte_common.h"
#include "mem_map.h"
#include "fpga_drv.h"
#include "fpga_net.h"
#include "fpga_tx.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/
#define TX_RING_MASK (TX_DESCRIPTOR_COUNT - 1)
#define FPGA_CONFIG  "/tmp/fpga_config"

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/
#define LOBYTE(w) ((uint8_t)(uint16_t)(w))
#define HIBYTE(w) ((uint8_t)(((uint16_t)(w)) >> 8))

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
static int tx_global_queue = TX_QUEUE_HIGIG;

static uint64_t tx_packet_num  = 0;
static uint32_t tx_dropped_num = 0;
/* static uint32_t tx_error_num = 0; */

static sem_t tx_sem;

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

    memset((uint8_t*)global_mem->base + TX_DESCRIPTOR_OFFSET, 0, TX_DESCRIPTOR_SIZE);

    if (access(FPGA_CONFIG, F_OK) != -1)
    {
        tx_global_queue = TX_QUEUE_FPGA_LOOP;
    }

    return sem_init(&tx_sem, 0, 1);
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
        DG_DBG_ERROR("%s(): unsupported tx queue: %d", __func__, queue);
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
//    static volatile uint32_t tx_tail = 0;

    uint32_t          head;
//    uint32_t          next;
    uint32_t          idx;
    tx_descp_entry_t* p_tx_desc = (tx_descp_entry_t*)((uint8_t*)global_mem->base + TX_DESCRIPTOR_OFFSET);
    uint64_t          reg       = 0;

    if (port >= FPGA_PORT_MAX)
    {
        DG_DBG_ERROR("TX specify an invalid port number: #%d", port);
        return -ENOTSUP;
    }

    tx_packet_num++;
    
    sem_wait(&tx_sem);
    head = tx_head;
//    next = head + 1;
    idx = head & TX_RING_MASK;

    /* check that we have tx entry available in ring */
    if (unlikely(p_tx_desc[idx].bufptr != NULL))
    {
        DG_DBG_ERROR("TX stuck while processing TX Descp #%d", idx);
        tx_dropped_num++;
        sem_post(&tx_sem);
        return -ENOBUFS;
    }

    /* test and increase tx_head atomically */
    tx_head++;
//    sem_post(&tx_sem);

    /* prepare mbuf data to tx */
    packet_buf_t* packet = (packet_buf_t*)((uint8_t*)global_mem->base + TX_MBUF_OFFSET + MBUF_SIZE * idx);
    setup_packet(packet, port, buf, len);
    p_tx_desc[idx].bufptr = (uint64_t*)packet;
    /* add the header length to packet */
    len += sizeof(packet_buf_t);

    /* prepare the register */
    reg  = INGOT_REGLB_LENGTH_SET(reg, len);
    reg  = INGOT_REGLB_QUEUE_SET(reg, tx_global_queue);
    reg |= idx;

    rte_compiler_barrier();

    /*
     * If there are other enqueues in progress that preceded us,
     * we need to wait for them to complete
     */
//    while (unlikely(tx_tail != head))
//    {
//        sched_yield();
//    }

    /* write to the fpga hardware */
    ingot_reg->tx_packet = reg;

//    tx_tail = next;
    sem_post(&tx_sem);
    return 0;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/
void setup_packet(packet_buf_t* packet, int port, const void* buf, size_t len)
{
    typedef struct
    {
        uint8_t port;
        uint8_t vlan;

    } bcm_port_info_t;

    static bcm_port_info_t bcm_port_map[FPGA_PORT_MAX] =
    {
#define BCM_PORT(x) [x] = { .port = BCM_ ## x, .vlan = (x + 1) * 10 }

        BCM_PORT(GE_0),
        BCM_PORT(GE_1),
        BCM_PORT(GE_2),
        BCM_PORT(GE_3),
        BCM_PORT(GE_4),
        BCM_PORT(GE_5),
        BCM_PORT(GE_6),
        BCM_PORT(GE_7),
        BCM_PORT(GE_8),
        BCM_PORT(GE_9),
        BCM_PORT(GE_10),
        BCM_PORT(GE_11),
        BCM_PORT(GE_12),
        BCM_PORT(GE_13),
        BCM_PORT(GE_14),
        BCM_PORT(GE_15),
        BCM_PORT(XE_0),
        BCM_PORT(XE_1),
        BCM_PORT(XE_2),
        BCM_PORT(XE_3),
        BCM_PORT(WTB_1),
        BCM_PORT(WTB_2)

#undef BCM_PORT
    };
    /* init the header */
    memset(packet, 0, sizeof(packet_buf_t));

    /* meta header */
    packet->meta.tx.transmit_queue = tx_global_queue;

    /* higig2 header */
    packet->hg2.start      = 0xfb;
    packet->hg2.tc         = 0x01;
    packet->hg2.src_mod    = 0x01;
    packet->hg2.dst_mod    = 0x00;
    packet->hg2.dst_port   = bcm_port_map[port].port;
    packet->hg2.src_port   = (tx_global_queue != TX_QUEUE_FPGA_LOOP) ? 0 : bcm_port_map[port].port;
    packet->hg2.lbid       = 0x09;
    packet->hg2.vlan_id_lo = LOBYTE(bcm_port_map[port].vlan);
    packet->hg2.vlan_id_hi = HIBYTE(bcm_port_map[port].vlan);
    packet->hg2.opcode     = 0x01;

    /* copy rest data to the tx mbuf */
    memcpy(packet->buf, buf, len);
}


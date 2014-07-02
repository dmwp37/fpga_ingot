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
#include "jspec/ingot.h"
#include "fpga_drv.h"
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
static inline void setup_packet(void* mbuf, int port, const void* buf, size_t len);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static volatile uint32_t tx_head = 0;

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init the FPGA tx driver
*//*==============================================================================================*/
void fpga_tx_init()
{
    phys_addr_t phys_base = global_mem->phys_addr;

    ingot_reg->tx_desc_base = phys_base + TX_DESCRIPTOR_OFFSET;
    ingot_reg->tx_buf_base  = phys_base + TX_MBUF_OFFSET;

    memset(global_mem->base + TX_DESCRIPTOR_OFFSET, 0, TX_DESCRIPTOR_SIZE);
}

/*=============================================================================================*//**
@brief transmit a packet with lock less multi thread support.

@param[in] port - which port to send packet
@param[in] buf  - the buffer contains the packet
@param[in] len  - packet buffer length

@return 0 if success
*//*==============================================================================================*/
int fpga_tx(int port, const void* buf, size_t len)
{
    uint32_t          head;
    uint32_t          idx;
    uint32_t          retry     = 0;
    void*             tx_mbuf   = global_mem->base + TX_MBUF_OFFSET;
    tx_descp_entry_t* p_tx_desc = global_mem->base + TX_DESCRIPTOR_OFFSET;
    uint64_t          reg       = 0;
    int               success;
    /* move tx_head atomically */
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
                return -ENOBUFS;
            }
            retry++;
            continue;
        }
        /* test and increase tx_head atomically */
        success = rte_atomic32_cmpset(&tx_head, head, head + 1);
    } while (unlikely(success == 0));

    /* prepare mbuf data to tx */
    tx_mbuf += MBUF_SIZE * idx;
    setup_packet(tx_mbuf, port, buf, len);
    p_tx_desc[idx].bufptr = (uint64_t*)tx_mbuf;


    /* prepare the register */
    reg = INGOT_REGLB_LENGTH_SET(reg, len);
    /* reg = INGOT_REGLB_QUEUE_SET(reg,TX_QUEUE); */
    reg  = INGOT_REGLB_QUEUE_SET(reg, port);
    reg |= idx;

    rte_compiler_barrier();

    /* write to the fpga hardware */
    ingot_reg->tx_packet = reg;

    return 0;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

void setup_packet(void* mbuf, int port, const void* buf, size_t len)
{
    meta_header_t meta = { 0 };
    meta.transmit_queue = port;

    /* add meta header */
    memcpy(mbuf, &meta, sizeof(meta));
    mbuf += sizeof(meta_header_t);

    /* add higig2 header */
    /* m_buf += sizeof(higig2_header_t); */

    /* copy rest data to the tx mbuf */
    memcpy(mbuf, buf, len);
}


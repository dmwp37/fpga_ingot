/*==================================================================================================

    Module Name:  fpga_rx.c

    General Description: Implements the FPGA rx driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "mem_map.h"
#include "rx_mbuf.h"
#include "jspec/ingot.h"
#include "fpga_drv.h"
#include "fpga_rx.h"
#include <dbg_dump.h>

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/
#define RX_RING_MASK (RX_DESCRIPTOR_COUNT - 1)

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static int fpga_rx_raw(rx_mbuf_t* rx_mbuf);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/
uint32_t rx_len_error = 0;

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static volatile uint32_t tx_head = 0;

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init the FPGA rx driver
*//*==============================================================================================*/
void fpga_rx_init()
{
    phys_addr_t phys_base = global_mem->phys_addr;

    ingot_reg->rx_desc_base = phys_base + RX_DESCRIPTOR_OFFSET;
    ingot_reg->rx_buf_base  = phys_base + RX_MBUF_OFFSET;

    memset(global_mem->base + RX_DESCRIPTOR_OFFSET, 0, RX_DESCRIPTOR_SIZE);
}

/*=============================================================================================*//**
@brief receive a packet from specified port.

@param[in] port - on which port to receive packet
@param[in] buf  - the buffer to receive packet
@param[in] len  - packet buffer length

@return length we have successfully received
*//*==============================================================================================*/
int fpga_rx(int port, void* buf, size_t len)
{
    int        ret;
    rx_mbuf_t* mbuf = NULL;

    if (port > RX_PORT_NUM)
    {
        printf("RX specify an invalid port number: #%d\n", port);
        return -ENOTSUP;
    }

    if ((mbuf = rx_port_get(port, 0)) == NULL)
    {
        return -ENOBUFS;
    }
    else
    {
        ret = mbuf->rx_head.buf_len;
    }

    return ret;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief receive a packet from FPGA.

@param[in] mbuf - the mbuf to receive packet

@return which port from
*//*==============================================================================================*/
int fpga_rx_raw(rx_mbuf_t* rx_mbuf)
{
    static volatile uint32_t rx_head = 0;

    uint32_t idx = rx_head & RX_RING_MASK;

    packet_buf_t* packet = (packet_buf_t*)(global_mem->base + RX_MBUF_OFFSET + MBUF_SIZE * idx);

    rx_descp_entry_t* p_rx_desc = global_mem->base + RX_DESCRIPTOR_OFFSET;

    int len = p_rx_desc[idx].buflen;
    int port;

    if (len == 0)
    {
        /* HW processing the descriptor or No Packets Received */
        return -EAGAIN;
    }

    DBG_PRINT(*packet);

    if (len < sizeof(packet_buf_t) + 16)
    {
        rx_len_error++;
        return -EAGAIN;
    }
    else
    {
        /* this is the port info */
        port = packet->hg2.src_port;
        /* store the len */
        len                     -= sizeof(packet_buf_t);
        rx_mbuf->rx_head.buf_len = len;
        /* Copy the data to the buffer provided to us */
        memcpy(rx_mbuf->buf, packet->buf, len);
    }

    /* Initialize the buflen of Descp to zero, so that HW will reuse it */
    p_rx_desc[idx].buflen = 0;
    /* bufprt will also be set to zero by FPGA */
    /* p_rx_desc[idx].bufptr = (uint64_t*)rx_mbuf; */

    rte_compiler_barrier();
    /*
     * Write the descriptor number to the rx_packet register to
     * inform the FPGA that the packet has been received and processed.
     */
    ingot_reg->rx_packet = idx;

    rx_head++;

    return port;
}

void fpga_rx_thread()
{
    /* the thread sould only run on one cpu with high priority */

    void* mbuf;
    int   port;

    while (1)
    {
        mbuf = rx_mbuf_get();
        while ((port = fpga_rx_raw(mbuf)) < 0)
        {
            /* wait until we got a valid mbuf */
        }

        /* put the mbuf to a port */
        rx_port_put(port, mbuf);
    }
}


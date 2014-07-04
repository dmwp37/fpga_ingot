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
static uint64_t rx_packet_num  = 0;
static uint32_t rx_dropped_num = 0;
static uint32_t rx_error_num   = 0;

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

    int len = p_rx_desc[idx].buflen; /* total packet length */
    int port;

    if (len == 0)
    {
        /* HW processing the descriptor or No Packets Received */
        return -EAGAIN;
    }

    DBG_PRINT(*packet);

    if (len < sizeof(packet_buf_t) + 16)
    {
        rx_error_num++;
        return -EAGAIN;
    }
    else
    {
        /* this is the port info */
        port = packet->hg2.src_port;
        /* stript the higig header */
        len -= sizeof(packet_buf_t);
        /* store the len */
        rx_mbuf->rx_head.buf_len = len;
        /* set the rx index */
        rx_mbuf->rx_head.rx_index = rx_packet_num;
        /* Copy the data to the buffer provided to us */
        memcpy(rx_mbuf->buf, packet->buf, len);

        rx_packet_num++;
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

    rx_mbuf_t* mbuf;
    int        port;

    while (1)
    {
        do
        {
            mbuf = rx_mbuf_get();
        } while (unlikely(mbuf == NULL));

        do
        {
            port = fpga_rx_raw(mbuf);

            if (port >= RX_PORT_NUM)
            {
                printf("rx packet port invalid, port=%d (discarded)", port);
                fflush(stdout);
                port = -1;
            }

        } while (port < 0); /* wait until we got a valid mbuf */

        /* print the data */
        DBG_PRINT(*mbuf);
        printf("rx packet data: 0x%" PRIx64 "\n", *(uint64_t*)mbuf->buf);
        fflush(stdout);

        /* put the mbuf to a port */
        if (unlikely(rx_port_put(port, mbuf) < 0))
        {
            rx_dropped_num++;
        }
    }
}


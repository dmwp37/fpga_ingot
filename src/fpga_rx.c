/*==================================================================================================

    Module Name:  fpga_rx.c

    General Description: Implements the FPGA rx driver interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include "dg_dbg.h"
#include "mem_map.h"
#include "rx_mbuf.h"
#include "fpga_drv.h"
#include "fpga_net.h"
#include "fpga_rx.h"

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
static int   map_bcm_port(int bcm_port);
static int   fpga_rx_raw(packet_buf_t* rx_mbuf);
static void* fpga_rx_thread_func(void* arg);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/
static uint64_t rx_packet_num     = 0;
static uint32_t rx_dropped_num    = 0;
static uint32_t rx_unexpected_num = 0;
static uint32_t rx_error_num      = 0;

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static volatile uint32_t tx_head = 0;

static pthread_t fpga_rx_thread;
static int       fpga_rx_thread_run = 0;

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init the FPGA rx driver

@return 0 if success
*//*==============================================================================================*/
int fpga_rx_init()
{
    phys_addr_t phys_base = global_mem->phys_addr;

    ingot_reg->rx_desc_base = phys_base + RX_DESCRIPTOR_OFFSET;
    ingot_reg->rx_buf_base  = phys_base + RX_MBUF_OFFSET;

    memset((uint8_t*)global_mem->base + RX_DESCRIPTOR_OFFSET, 0, RX_DESCRIPTOR_SIZE);

    fpga_rx_thread_run = 1;
    if (pthread_create(&fpga_rx_thread, NULL, fpga_rx_thread_func, NULL) != 0)
    {
        DG_DBG_ERROR("could not create fpga rx thread!");
        fpga_rx_thread_run = 0;
        return -1;
    }

    return 0;
}


/*=============================================================================================*//**
@brief exit the FPGA rx driver
*//*==============================================================================================*/
void fpga_rx_exit()
{
    if (fpga_rx_thread_run != 0)
    {
        /* stop the rx thread */
        fpga_rx_thread_run = 0;
        pthread_join(fpga_rx_thread, NULL);
    }
}

/*=============================================================================================*//**
@brief receive a packet from specified port.

@param[in] port - on which port to receive packet
@param[in] buf  - the buffer to receive packet
@param[in] len  - packet buffer length

@return length we have successfully received
*//*==============================================================================================*/
int fpga_net_rx(fpga_net_port_t port, void* buf, size_t len)
{
    size_t        ret;
    packet_buf_t* mbuf = NULL;

    if (port >= FPGA_PORT_MAX)
    {
        DG_DBG_ERROR("RX specify an invalid port number: #%d", port);
        return -ENOTSUP;
    }

    if ((mbuf = rx_port_get(port, 1500)) == NULL)
    {
        return -ENOBUFS;
    }

    ret = mbuf->meta.rx.buf_len;
    ret = (len > ret) ? ret : len;

    /* copy the data to user */
    memcpy(buf, mbuf->buf, ret);

    /* release the mbuf */
    rx_mbuf_put(mbuf);

    return ret;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/
int map_bcm_port(int bcm_port)
{
#define PORT_MAPPER(x) case BCM_ ## x: return x
    switch (bcm_port)
    {
        PORT_MAPPER(GE_0);
        PORT_MAPPER(GE_1);
        PORT_MAPPER(GE_2);
        PORT_MAPPER(GE_3);
        PORT_MAPPER(GE_4);
        PORT_MAPPER(GE_5);
        PORT_MAPPER(GE_6);
        PORT_MAPPER(GE_7);
        PORT_MAPPER(GE_8);
        PORT_MAPPER(GE_9);
        PORT_MAPPER(GE_10);
        PORT_MAPPER(GE_11);
        PORT_MAPPER(GE_12);
        PORT_MAPPER(GE_13);
        PORT_MAPPER(GE_14);
        PORT_MAPPER(GE_15);
        PORT_MAPPER(XE_0);
        PORT_MAPPER(XE_1);
        PORT_MAPPER(XE_2);
        PORT_MAPPER(XE_3);
        PORT_MAPPER(WTB_1);
        PORT_MAPPER(WTB_2);
    }
#undef PORT_MAPPER
    return -1;
}

/*=============================================================================================*//**
@brief receive a packet from FPGA.

@param[in] mbuf - the mbuf to receive packet

@return which bcm port from
*//*==============================================================================================*/
int fpga_rx_raw(packet_buf_t* rx_mbuf)
{
    static volatile uint32_t rx_head = 0;

    uint32_t idx = rx_head & RX_RING_MASK;

    packet_buf_t* packet = (packet_buf_t*)((uint8_t*)global_mem->base +
                                           RX_MBUF_OFFSET + MBUF_SIZE * idx);

    rx_descp_entry_t* p_rx_desc = (rx_descp_entry_t*)((uint8_t*)global_mem->base +
                                                      RX_DESCRIPTOR_OFFSET);

    size_t len = p_rx_desc[idx].buflen; /* total packet length */
    int    port;

    if (len == 0)
    {
        /* HW processing the descriptor or No Packets Received */
        return -EAGAIN;
    }

    /* GDB_PRINT(*packet); */
    /* DG_DBG_ERROR("get a packet from bcm_port[%d]", packet->hg2.src_port); */

    /* here we do some packet filter */
    /* this is the port info */
    int                  bcm_port = packet->hg2.src_port;
    struct ether_header* eh       = (struct ether_header*)packet->buf;

    if (unlikely((port = map_bcm_port(bcm_port)) < 0))
    {
        DG_DBG_TRACE("rx packet port invalid, bcm_port=%d (discarded)", bcm_port);
        rx_unexpected_num++;
    }
    else if (unlikely(len < (sizeof(packet_buf_t) + FPGA_NET_PACKET_LEN_MIN)))
    {
        DG_DBG_TRACE("rx packet length invalid, len=%d (discarded)", len);
        rx_error_num++;
        port = -EAGAIN;
    }
    else if (unlikely(eh->ether_type != ntohs(ETH_P_DIAG)))
    {
        /* filter out DIAG packet */
        DG_DBG_TRACE("rx packet type invalid, port=%d (discarded)", port);
        rx_unexpected_num++;
        port = -EAGAIN;
    }
    else
    {
        /* Copy the packet */
        memcpy(rx_mbuf, packet, len);
        /* store the len without header */
        rx_mbuf->meta.rx.buf_len = len - sizeof(packet_buf_t);
        /* set the rx index */
        rx_mbuf->meta.rx.rx_index = rx_packet_num;
    }

    rx_packet_num++;

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

/*=============================================================================================*//**
@brief fpga rx thread function
*//*==============================================================================================*/
void* fpga_rx_thread_func(void* arg __attribute__((__unused__)))
{
#ifdef __linux__
    /* the thread should only run on one cpu with high priority */
    long      cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    cpu_set_t cpu_info;
    CPU_ZERO(&cpu_info);
    CPU_SET(cpu_num - 1, &cpu_info);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_info) != 0)
    {
        DG_DBG_ERROR("set rx thread affinity failed");
    }
#endif

    packet_buf_t* mbuf;
    int           port;

    while (fpga_rx_thread_run)
    {
        do
        {
            if (unlikely(fpga_rx_thread_run == 0))
            {
                return NULL;
            }

            mbuf = rx_mbuf_get();
        } while (unlikely(mbuf == NULL));

        do
        {
            if (unlikely(fpga_rx_thread_run == 0))
            {
                return NULL;
            }

            port = fpga_rx_raw(mbuf);
#if 0
            if (port > 0)
            {
                DG_DBG_ERROR("get a packet from port[%d]", port);
            }
#endif

        } while (port < 0); /* wait until we got a valid mbuf */

        /* print the data */
        /* GDB_PRINT(*mbuf); */

        /* put the mbuf to a port */
        if (unlikely(rx_port_put(port, mbuf) < 0))
        {
            rx_dropped_num++;
        }
    }

    return NULL;
}


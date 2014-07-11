#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include "mem_map.h"
#include "fpga_drv.h"
#include "fpga_net.h"
#include <gdb_print.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

/* jit_checksum:
 * Calulate the checksum
 */
u_int16_t jit_checksum(u_int8_t* ptr, u_int16_t len)
{
    u_int32_t  sum;
    u_int16_t* ptr_16 = (u_int16_t*)ptr;
    sum = 0;
    for (len = len >> 1; len > 0; len--)
    {
        sum += htons(*ptr_16++);
    }

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16) & (0xffff);
    return (u_int16_t) ~sum;
}

/*
 * jit_setup_ip_hdr:
 * Build the ip header for outgoing message.
 */

static void jit_setup_ip_hdr(struct ip* ip_hdr, u_int16_t length)
{
    u_int16_t res;

    ip_hdr->ip_len = htons(length + sizeof(struct ip));
    ip_hdr->ip_sum = 0;
    res            = jit_checksum((u_int8_t*)ip_hdr, sizeof(struct ip));

    ip_hdr->ip_sum = htons(res);
}


void test_fpga_net()
{
    if (fpga_net_init() != 0)
    {
        printf("fpga net driver init failed.\n");
        return;
    }

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", fpga_drv_get_version());

    uint8_t tx_data[1024];
    uint8_t rx_data[1024];

    struct ether_header* eh         = (struct ether_header*)tx_data;
    unsigned char        mac_src[6] = { 0x00, 0xDE, 0xB0, 0x10, 0x86, 0x00 };
    unsigned char        mac_dst[6] = { 0x00, 0xDE, 0xB0, 0x10, 0x82, 0x00 };

    /* Construct the Ethernet header */
    memset(eh, 0, ETH_HLEN);
    memcpy(eh->ether_shost, mac_src, 6);
    memcpy(eh->ether_dhost, mac_dst, 6);
    eh->ether_type = htons(ETHERTYPE_IP);

    struct ip* ip_header = (struct ip*)(tx_data + sizeof(struct ether_header));
    jit_setup_ip_hdr(ip_header, sizeof(tx_data) - sizeof(struct ip) - 32 - sizeof(struct ether_header));


    fpga_net_port_t port = WTB_2;
    int             i;
    for (i = 0; i < 100; i++)
    {
        for (port = 0; port < FPGA_PORT_MAX; port++)
        {
            fpga_net_tx(port, tx_data, sizeof(tx_data));

            if (fpga_net_rx(port, rx_data, sizeof(rx_data)) < 0)
            {
                printf("test faied\n");
                exit(1);
            }
        }
    }

    printf("test passed\n");

/* tx_descp_entry_t* p_tx_desc = (tx_descp_entry_t*)((uint8_t*)global_mem->base + TX_DESCRIPTOR_OFFSET); */
/* GDB_PRINT(*global_mem); */
/* for(i=0; i<64; i++) */
/* GDB_PRINT(p_tx_desc[i]); */
/* (void)p_tx_desc; */
}

int main()
{
    test_fpga_net();

    return 0;
}


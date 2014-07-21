#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include "dg_dbg.h"
#include "mem_map.h"
#include "fpga_drv.h"
#include "fpga_net.h"
#include "rx_mbuf.h"

#define FPGA_CONFIG "/tmp/fpga_config"

void test_fpga_net()
{
    if (fpga_net_init() != 0)
    {
        printf("fpga net driver init failed.\n");
        return;
    }

    fpga_net_config(TX_QUEUE_FPGA_LOOP);

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", fpga_drv_get_version());

    uint8_t tx_data[1024];
    uint8_t rx_data[1024];

    struct ether_header* eh         = (struct ether_header*)tx_data;
    unsigned char        mac_dst[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    /* Construct the Ethernet header */
    memset(eh, 0, ETH_HLEN);
    memcpy(eh->ether_dhost, mac_dst, 6);
    eh->ether_type = htons(ETH_P_DIAG);

    fpga_net_port_t port = GE_1;
    int             i;
    for (i = 0; i < 100; i++)
    {
        for (port = 0; port < FPGA_PORT_MAX; port++)
        {
            tx_data[8]  = i;
            tx_data[10] = port;
            fpga_net_tx(port, tx_data, sizeof(tx_data));

            if (fpga_net_rx(port, rx_data, sizeof(rx_data)) < 0)
            {
                printf("fpga test faied\n");
                exit(1);
            }
            else if (tx_data[8] != i || rx_data[10] != port)
            {
                printf("fpga test receive wrong data\n");
                exit(1);
            }
        }
    }

    i = DG_DBG_get_dbg_level();
    DG_DBG_set_dbg_level(DG_DBG_LVL_DISABLE);
    for (port = 0; port < FPGA_PORT_MAX; port++)
    {
        if (rx_port_get(port, 0) != NULL)
        {
            printf("fpga rx multi-cast!\n");
            exit(1);
        }
    }
    DG_DBG_set_dbg_level(i);

    printf("fpga test passed\n");

#if 0
    tx_descp_entry_t* p_tx_desc = (tx_descp_entry_t*)((uint8_t*)global_mem->base + TX_DESCRIPTOR_OFFSET);
    GDB_PRINT(*global_mem);
    for (i = 0; i < 64; i++)
    {
        GDB_PRINT(p_tx_desc[i]);
    }
    (void)p_tx_desc;
#endif
}

int main(int argc, char** argv)
{
    if (argc >= 2 && argv[1][0] == '0')
    {
        FILE* cfg_file = fopen(FPGA_CONFIG, "w+");
        if (cfg_file != NULL)
        {
            fprintf(cfg_file, "%s\n", argv[1]);
            fclose(cfg_file);
        }
    }
    else
    {
        unlink(FPGA_CONFIG);
    }
    test_fpga_net();

    return 0;
}


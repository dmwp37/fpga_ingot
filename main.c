#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fpga_drv.h"
#include "fpga_net.h"


void test_fpga_net()
{
    if (fpga_net_init() != 0)
    {
        printf("fpga net driver init failed.\n");
        return;
    }

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", fpga_drv_get_version());

    uint64_t tx_data[128];
    uint64_t rx_data[128];

    fpga_net_port_t port;
    for (port = 0; port < FPGA_PORT_MAX; port++)
    {
        tx_data[0] = 0xaabbccddeeffdead;
        fpga_net_tx(port, tx_data, sizeof(tx_data));
        tx_data[0] = 0xcccccccccccccccc;
        fpga_net_tx(port, tx_data, sizeof(tx_data));

        fpga_net_rx(port, rx_data, sizeof(rx_data));
        printf("port[%d] rx packet data: 0x%" PRIx64 "\n", port, rx_data[0]);
        fpga_net_rx(port, rx_data, sizeof(rx_data));
        printf("port[%d] rx packet data: 0x%" PRIx64 "\n", port, rx_data[0]);
    }
}

int main()
{
    test_fpga_net();

    return 0;
}


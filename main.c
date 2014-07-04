#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbg_dump.h>
#include "uio.h"
#include "fpga_drv.h"
#include "hp_malloc.h"
#include "mem_map.h"
#include "rx_mbuf.h"
#include "fpga_tx.h"
#include "fpga_rx.h"

void test_fpga_uio()
{
    if (fpga_drv_init() < 0)
    {
        printf("error: fpga uio init failed\n");
        exit(1);
    }
    fpga_drv_reset();

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", fpga_drv_get_version());

    mem_map_init();
    rx_mbuf_init();
    fpga_rx_init();
    fpga_tx_init();

    uint64_t* p = global_mem->base + MEM_END;

    *p = 0xaabbccddeeffdead;
    fpga_tx(0, p, 1024);
    *p = 0xcccccccccccccccc;
    fpga_tx(0, p, 1024);

    p += MBUF_SIZE;

    fpga_rx(0, p, 1024);
    fpga_rx(0, p, 1024);

    fpga_rx_exit();
    rx_mbuf_exit();
    fpga_drv_exit();
    mem_map_exit();
}

int main()
{
    dump_mem_map();
    test_fpga_uio();

    return 0;
}


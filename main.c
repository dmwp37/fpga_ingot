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

void test_fpga_tx()
{
    fpga_tx_init();

    fpga_tx(global_mem->base, 1024);
}

void test_rx_mbuf()
{
    rx_mbuf_init();

    void* mbuf = rx_mbuf_get();

    rx_mbuf_put(mbuf);
}

void test_hp_malloc()
{
    hp_t* p_hp = hp_alloc(128 * 1024);

    DBG_PRINT(sizeof(struct rte_ring));

    if (p_hp == NULL)
    {
        printf("error: alloc huge pages failed\n");
        exit(1);
    }

    DBG_PRINT(*p_hp);

    hp_free(p_hp);
}

void test_fpga_uio()
{
    if (fpga_drv_init() < 0)
    {
        printf("error: fpga uio init failed\n");
        exit(1);
    }

    fpga_drv_reset();

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", fpga_drv_get_version());

    fpga_drv_exit();
}

int main()
{
    dump_mem_map();
    mem_map_init();
    test_hp_malloc();
    test_rx_mbuf();
    test_fpga_uio();
    test_fpga_tx();
    mem_map_exit();
    return 0;
}


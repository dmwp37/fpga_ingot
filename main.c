#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbg_dump.h>
#include "uio.h"
#include "fpga_drv.h"
#include "hp_malloc.h"
#include "mbuf.h"

void test_mbuf()
{
    mbuf_pool_t* pool = mbuf_pool_alloc(1024);

    DBG_PRINT(*pool);

    void* mbuf = mbuf_get(pool);

    mbuf_put(pool, mbuf);

    mbuf_pool_free(pool);
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
    test_hp_malloc();
    test_mbuf();
    test_fpga_uio();
    return 0;
}


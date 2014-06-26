#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbg_dump.h>
#include "uio.h"
#include "fpga_drv.h"
#include "hp_malloc.h"

void test_hp_malloc()
{
    hp_t* p_hp = malloc_huge_pages(128 * 1024);

    if (p_hp == NULL)
    {
        printf("error: alloc huge pages failed\n");
        exit(1);
    }

    DBG_PRINT(*p_hp);

    free_huge_pages(p_hp);
}

void test_fpga_uio()
{
    if (!fpga_drv_init())
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
    test_fpga_uio();
    return 0;
}


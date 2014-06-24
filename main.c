#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fpga_uio.h"
#include "fpga_drv.h"

int main()
{
    if (!FPGA_UIO_init())
    {
        printf("error: fpga uio init failed\n");
        exit(1);
    }

    FPGA_DRV_reset();

    printf("FPGA ingot version is: 0x%" PRIx64 "\n", FPGA_DRV_get_version());

    FPGA_UIO_exit();

    return 0;
}


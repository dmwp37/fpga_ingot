#include <stdio.h>
#include "dump_struct.h"
#include "fpga_uio.h"

int main()
{

    FPGA_UIO_init();

    FPGA_UIO_reset();

    FPGA_UIO_exit();
    return 0;
}


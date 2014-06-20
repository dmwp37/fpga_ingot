#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbg_dump.h"
#include "fpga_uio.h"

int main()
{
    ingot_t* p = malloc(sizeof(ingot_t));
    
    FPGA_UIO_init();

    FPGA_UIO_reset();

//    memcpy(p, FPGA_UIO_get_reg(), sizeof(ingot_t));
    DBG_PRINT(*p);

    FPGA_UIO_exit();
    
    free(p);
    return 0;
}


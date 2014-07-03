/*==================================================================================================

    Module Name:  mem_map.c

    General Description: global memory management

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "mem_map.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/
#define print_macro(x) printf(# x ": %p\n", (void*)x)

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/
hp_t* global_mem  = NULL;
hp_t* rx_mbuf_mem = NULL;

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief dump the memory map
*//*==============================================================================================*/
void dump_mem_map()
{
    print_macro(TX_DESCRIPTOR_OFFSET);
    print_macro(TX_DESCRIPTOR_SIZE);
    print_macro(TX_DESCRIPTOR_END);
    printf("\n");

    print_macro(TX_MBUF_OFFSET);
    print_macro(TX_MBUF_SIZE);
    print_macro(TX_MBUF_END);
    printf("\n");

    print_macro(RX_DESCRIPTOR_OFFSET);
    print_macro(RX_DESCRIPTOR_SIZE);
    print_macro(RX_DESCRIPTOR_END);
    printf("\n");

    print_macro(RX_MBUF_OFFSET);
    print_macro(RX_MBUF_SIZE);
    print_macro(RX_MBUF_END);
    printf("\n");

    print_macro(RX_MBUF_RING_OFFSET);
    print_macro(RX_MBUF_RING_SIZE);
    print_macro(RX_MBUF_RING_END);
    printf("\n");

    print_macro(RX_PORT_RING_OFFSET);
    print_macro(RX_PORT_RING_SIZE);
    print_macro(RX_PORT_RING_END);
    print_macro(RX_PORT_RING_REAL_END);
    print_macro(RX_PORT_RING_REAL_SIZE);
    print_macro(RX_PORT_RING_TOTAL_SIZE);
    printf("\n");

    print_macro(MEM_END);
    printf("\n");
}

/*=============================================================================================*//**
@brief init global memory map

@param[in] count - how many mbuf the pool contains, each mbuf is a 2k bytes buffer

@return pointer to mbuf_pool
*//*==============================================================================================*/
void mem_map_init()
{
    global_mem = hp_alloc(TOTAL_MEM_SIZE);
    if (global_mem == NULL)
    {
        printf("%s(): can't alloc global memory!\n", __func__);
        exit(1);
    }

    rx_mbuf_mem = hp_alloc(MBUF_SIZE * RX_MBUF_COUNT);
    if (rx_mbuf_mem == NULL)
    {
        printf("%s(): can't alloc rx mbuf!\n", __func__);
        exit(1);
    }
}


/*=============================================================================================*//**
@brief free global memory map

@param[in] pool - pointer to mbuf pool
*//*==============================================================================================*/
void mem_map_exit()
{
    hp_free(global_mem);
    hp_free(rx_mbuf_mem);

    global_mem  = NULL;
    rx_mbuf_mem = NULL;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/


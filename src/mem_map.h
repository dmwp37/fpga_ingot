#ifndef _MEM_MAP_H_
#define _MEM_MAP_H_
/*==================================================================================================

    Module Name:  mem_map.h

    General Description: This file contains the memory map definition

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <inttypes.h>
#include "net_stack.h"
#include "hp_malloc.h"
#include "rte_ring.h"
#include "fpga_net.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                              MACROS
==================================================================================================*/
#ifdef RTE_ALIGN
#undef RTE_ALIGN
#define RTE_ALIGN(x, y) (x) /*speed up processing */
#endif

#define MBUF_SIZE               (2048) /*each mbuf size must be 2k bytes */
#define DMA_ALIGN               (256)
#define TX_DESCRIPTOR_COUNT     (64)
#define RX_DESCRIPTOR_COUNT     (64)
#define TX_MBUF_COUNT           (TX_DESCRIPTOR_COUNT)
#define RX_PORT_MBUF_COUNT      (4096) /* 4k mbuf (8M) for each rx port */
#define RX_PORT_NUM             (FPGA_PORT_MAX)
#define RX_MBUF_COUNT           (RX_PORT_MBUF_COUNT * 16) /* must be power of 2 (128M bytes) */
#define RTE_RING_SIZE(count)    (sizeof(struct rte_ring) + count * sizeof(void*))
/* each memory zone should define offset, size, and end */
#define MEM_BEGIN               (0)
/* tx descriptor */
#define TX_DESCRIPTOR_OFFSET    (RTE_ALIGN(MEM_BEGIN, DMA_ALIGN))
#define TX_DESCRIPTOR_SIZE      (TX_DESCRIPTOR_COUNT * sizeof(tx_descp_entry_t))
#define TX_DESCRIPTOR_END       (TX_DESCRIPTOR_OFFSET + TX_DESCRIPTOR_SIZE)
/* tx mbuf */
#define TX_MBUF_OFFSET          (RTE_ALIGN(TX_DESCRIPTOR_END, DMA_ALIGN))
#define TX_MBUF_SIZE            (MBUF_SIZE * TX_DESCRIPTOR_COUNT)
#define TX_MBUF_END             (TX_MBUF_OFFSET + TX_MBUF_SIZE)
/* rx descriptor */
#define RX_DESCRIPTOR_OFFSET    (RTE_ALIGN(TX_MBUF_END, DMA_ALIGN))
#define RX_DESCRIPTOR_SIZE      (RX_DESCRIPTOR_COUNT * sizeof(rx_descp_entry_t))
#define RX_DESCRIPTOR_END       (RX_DESCRIPTOR_OFFSET + RX_DESCRIPTOR_SIZE)
/* rx descriptor */
#define RX_MBUF_OFFSET          (RTE_ALIGN(RX_DESCRIPTOR_END, DMA_ALIGN))
#define RX_MBUF_SIZE            (MBUF_SIZE * RX_DESCRIPTOR_COUNT)
#define RX_MBUF_END             (RX_MBUF_OFFSET + RX_MBUF_SIZE)
/* rx global mbuf ring for cache */
#define RX_MBUF_RING_OFFSET     (RTE_ALIGN(RX_MBUF_END, CACHE_LINE_SIZE))
#define RX_MBUF_RING_SIZE       (RTE_RING_SIZE(RX_MBUF_COUNT))
#define RX_MBUF_RING_END        (RX_MBUF_RING_OFFSET + RX_MBUF_RING_SIZE)
/* rx port ring */
#define RX_PORT_RING_OFFSET     (RTE_ALIGN(RX_MBUF_RING_END, CACHE_LINE_SIZE))
#define RX_PORT_RING_SIZE       (RTE_RING_SIZE(RX_PORT_MBUF_COUNT))
#define RX_PORT_RING_END        (RX_PORT_RING_OFFSET + RX_PORT_RING_SIZE)
#define RX_PORT_RING_REAL_END   (RTE_ALIGN(RX_PORT_RING_END, CACHE_LINE_SIZE))
#define RX_PORT_RING_REAL_SIZE  (RX_PORT_RING_REAL_END - RX_PORT_RING_OFFSET)
#define RX_PORT_RING_TOTAL_SIZE (RX_PORT_RING_REAL_SIZE * RX_PORT_NUM)

#define MEM_END                 (RX_PORT_RING_OFFSET + RX_PORT_RING_TOTAL_SIZE)

#define TOTAL_MEM_SIZE          (MEM_END - MEM_BEGIN)

/*==================================================================================================
                                               ENUMS
==================================================================================================*/

/*==================================================================================================
                                   STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
                                   GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
extern hp_t* global_mem;

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/
int  mem_map_init();
void mem_map_exit();
void dump_mem_map();


#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _MEM_MAP_H_  */


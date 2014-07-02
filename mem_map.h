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
#include "hp_malloc.h"
#include "rte_ring.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                              MACROS
==================================================================================================*/
#define MBUF_SIZE            (2048) /*each mbuf size must be 2k bytes */
#define DMA_ALIGN            (256)
#define TX_DESCRIPTOR_COUNT  (64)
#define RX_DESCRIPTOR_COUNT  (64)
#define TX_MBUF_COUNT        (TX_DESCRIPTOR_COUNT)
#define RX_PORT_MBUF_COUNT   (4096) /* 4k mbuf (8M) for each rx port */
#define RX_PORT_NUM          (20)
#define RX_MBUF_COUNT        (RX_PORT_MBUF_COUNT * 16) /* must be power of 2 (128M bytes) */

#define RTE_RING_SIZE(count) (sizeof(struct rte_ring) + count * sizeof(void*))


/* each memory zone should define offset, size, and end */
#define MEM_BEGIN               (0)

#define TX_DESCRIPTOR_OFFSET    (RTE_ALIGN(MEM_BEGIN, DMA_ALIGN))
#define TX_DESCRIPTOR_SIZE      (TX_DESCRIPTOR_COUNT * sizeof(phys_addr_t))
#define TX_DESCRIPTOR_END       (TX_DESCRIPTOR_OFFSET + TX_DESCRIPTOR_SIZE)

#define TX_MBUF_OFFSET          (RTE_ALIGN(TX_DESCRIPTOR_END, DMA_ALIGN))
#define TX_MBUF_SIZE            (MBUF_SIZE * TX_DESCRIPTOR_COUNT)
#define TX_MBUF_END             (TX_MBUF_OFFSET + TX_MBUF_SIZE)

#define RX_DESCRIPTOR_OFFSET    (RTE_ALIGN(TX_MBUF_END, DMA_ALIGN))
#define RX_DESCRIPTOR_SIZE      (RX_DESCRIPTOR_COUNT * sizeof(phys_addr_t))
#define RX_DESCRIPTOR_END       (RX_DESCRIPTOR_OFFSET + RX_DESCRIPTOR_SIZE)

#define RX_MBUF_RING_OFFSET     (RTE_ALIGN(RX_DESCRIPTOR_END, CACHE_LINE_SIZE))
#define RX_MBUF_RING_SIZE       (RTE_RING_SIZE(RX_MBUF_COUNT))
#define RX_MBUF_RING_END        (RX_MBUF_RING_OFFSET + RX_MBUF_RING_SIZE)

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
extern hp_t* rx_mbuf_mem;

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/
void dump_mem_map();
void mem_map_init();
void mem_map_exit();


#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* _MEM_MAP_H_  */


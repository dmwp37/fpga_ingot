/*==================================================================================================

    Module Name:  rx_mbuf.c

    General Description: Implements the rx_mbuf management

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include <errno.h>

#include "dg_sem.h"
#include "dg_dbg.h"
#include "mem_map.h"
#include "rx_mbuf.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
typedef struct
{
    struct rte_ring* ring;
    dg_sem_t         sem;
} rx_port_ring_t;

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static hp_t*            rx_mbuf_mem  = NULL;
static struct rte_ring* rx_mbuf_ring = NULL;

static rx_port_ring_t rx_port_ring[RX_PORT_NUM];

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief init rx mbuf rte_rings

@return 0 if success
*//*==============================================================================================*/
int rx_mbuf_init()
{
    int      i;
    int      ret = 0;
    uint8_t* p_mbuf;

    rx_mbuf_ring = (struct rte_ring*)((uint8_t*)global_mem->base + RX_MBUF_RING_OFFSET);

    rx_mbuf_mem = hp_alloc(MBUF_SIZE * RX_MBUF_COUNT);
    if (rx_mbuf_mem == NULL)
    {
        DG_DBG_ERROR("%s(): can't alloc rx mbuf!", __func__);
        return -1;
    }

    rte_ring_init(rx_mbuf_ring, RX_MBUF_COUNT);

    p_mbuf = rx_mbuf_mem->base;
    for (i = 0; i < RX_MBUF_COUNT; i++, p_mbuf += MBUF_SIZE)
    {
        rx_mbuf_ring->ring[i] = p_mbuf;
    }

    /* till now the ring should be full */
    rx_mbuf_ring->prod.head += RX_MBUF_COUNT - 1;
    rx_mbuf_ring->prod.tail += RX_MBUF_COUNT - 1;

    /* init the port mbuf ring */
    uint8_t* port_ring = (uint8_t*)global_mem->base + RX_PORT_RING_OFFSET;
    for (i = 0; i < RX_PORT_NUM; i++)
    {
        /* init the semaphore for blocking read */
        dg_sem_init(&rx_port_ring[i].sem, 0);

        rx_port_ring[i].ring = (struct rte_ring*)port_ring;
        rte_ring_init((struct rte_ring*)port_ring, RX_PORT_MBUF_COUNT);

        port_ring += RX_PORT_RING_REAL_SIZE;
    }

    return ret;
}

/*=============================================================================================*//**
@brief exit rx mbuf rings
*//*==============================================================================================*/
void rx_mbuf_exit()
{
    hp_free(rx_mbuf_mem);
    rx_mbuf_mem = NULL;
}

/*=============================================================================================*//**
@brief get one mbuf

@return rx_mbuf pointer
*//*==============================================================================================*/
void* rx_mbuf_get()
{
    void* mbuf = NULL;

    /* this is sc dequeue for the rx thread, not mc */
    if (unlikely(rte_ring_sc_dequeue(rx_mbuf_ring, &mbuf) != 0))
    {
        DG_DBG_ERROR("%s(): no mbuf available in the mbuf pool!", __func__);
        return NULL;
    }

    return mbuf;
}


/*=============================================================================================*//**
@brief free one mbuf

@param[in] mbuf - pointer to one mbuf
*//*==============================================================================================*/
void rx_mbuf_put(void* mbuf)
{
    if (unlikely(rte_ring_mp_enqueue_s(rx_mbuf_ring, mbuf) != 0))
    {
        DG_DBG_ERROR("%s(): can't enqueue mbuf to the mbuf pool!", __func__);
    }
}

/*=============================================================================================*//**
@brief push one rx mbuf to the port ring buf

@param[in] port - port number
@param[in] mbuf - pointer to one mbuf

@return 0 if success
*//*==============================================================================================*/
int rx_port_put(int port, void* mbuf)
{
    if (unlikely(rte_ring_sp_enqueue(rx_port_ring[port].ring, mbuf) != 0))
    {
        DG_DBG_ERROR("%s(): can't enqueue mbuf to the port[%d] ring!", __func__, port);
        return -1;
    }

    dg_sem_post(&rx_port_ring[port].sem);

    return 0;
}

/*=============================================================================================*//**
@brief get one mbuf from a specified port

@param[in] port - port number
@param[in] time - how long should wait to get data in ms

@return rx_mbuf pointer
*//*==============================================================================================*/
void* rx_port_get(int port, int time)
{
    void* mbuf = NULL;

    if (dg_sem_wait(&rx_port_ring[port].sem, time) == 0)
    {
        if (unlikely(rte_ring_mc_dequeue_s(rx_port_ring[port].ring, &mbuf) != 0))
        {
            DG_DBG_ERROR("%s(): no mbuf available in port[%d]", __func__, port);
            DG_DBG_ERROR("%s(): sem sync wrong!", __func__);
            exit(1);
        }
    }
    else
    {
        DG_DBG_ERROR("%s(): no mbuf available in port[%d]", __func__, port);
    }

    return mbuf;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/


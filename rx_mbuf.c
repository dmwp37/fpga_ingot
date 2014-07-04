/*==================================================================================================

    Module Name:  rx_mbuf.c

    General Description: Implements the rx_mbuf management

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/time.h>
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
    sem_t            sem;
} rx_port_ring_t;

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static int wait_sem(sem_t* sem, int time_out);

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
*//*==============================================================================================*/
void rx_mbuf_init()
{
    int   i;
    void* p_mbuf;

    rx_mbuf_ring = global_mem->base + RX_MBUF_RING_OFFSET;

    rx_mbuf_mem = hp_alloc(MBUF_SIZE * RX_MBUF_COUNT);
    if (rx_mbuf_mem == NULL)
    {
        printf("%s(): can't alloc rx mbuf!\n", __func__);
        exit(1);
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
    void* port_ring = global_mem->base + RX_PORT_RING_OFFSET;
    for (i = 0; i < RX_PORT_NUM; i++)
    {
        /* init the semaphore for blocking read */
        if (sem_init(&rx_port_ring[i].sem, 0, 0) != 0)
        {
            printf("Failed to init semphore for rx port, errno=%d(%m)", errno);
            exit(1);
        }

        rx_port_ring[i].ring = port_ring;
        rte_ring_init(port_ring, RX_PORT_MBUF_COUNT);

        port_ring += RX_PORT_RING_REAL_SIZE;
    }
}

/*=============================================================================================*//**
@brief exit rx mbuf rings
*//*==============================================================================================*/
void rx_mbuf_exit()
{
    int i;

    for (i = 0; i < RX_PORT_NUM; i++)
    {
        sem_destroy(&rx_port_ring[i].sem);
    }

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
        printf("%s(): no mbuf available in the mbuf pool!\n", __func__);
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
    if (unlikely(rte_ring_mp_enqueue(rx_mbuf_ring, mbuf) != 0))
    {
        printf("%s(): can't enqueue mbuf to the mbuf pool!\n", __func__);
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
        printf("%s(): can't enqueue mbuf to the port[%d] ring!\n", __func__, port);
        return -1;
    }

    sem_post(&rx_port_ring[port].sem);

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

    if (wait_sem(&rx_port_ring[port].sem, time) == 0)
    {
        if (unlikely(rte_ring_mc_dequeue(rx_port_ring[port].ring, &mbuf) != 0))
        {
            printf("%s(): no mbuf available in the port[%d] mbuf !\n", __func__, port);
            printf("%s(): sem sync wrong!\n", __func__);
            exit(1);
        }
    }

    return mbuf;
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief Wait the semaphore for specified time

@param[in] sem     - the pointer of semaphore
@param[in] timeout - timeout value to wait in ms

@return 0 for success

@note
 - if timeout is 0, no wait
 - -1, wait for ever
*//*==============================================================================================*/
int wait_sem(sem_t* sem, int time_out)
{
    int             status = 0;
    struct timeval  time_of_day;
    struct timespec timeout_time;

    if (time_out > 0)
    {
        if (gettimeofday(&time_of_day, NULL) != 0)
        {
            printf("Failed to get time of day, errno=%d(%m)\n", errno);
            return -1;
        }
        else
        {
            /* Add the timeout time to the time of day to get absolute timeout time */
            timeout_time.tv_sec  = time_of_day.tv_sec;
            timeout_time.tv_nsec = time_of_day.tv_usec * 1000;

            timeout_time.tv_sec  += time_out / 1000;
            timeout_time.tv_nsec += (time_out % 1000) * 1000000;
        }
    }

    if (time_out < 0)
    {
        status = sem_wait(sem);
    }
    else if (time_out == 0)
    {
        status = sem_trywait(sem);
    }
    else
    {
        status = sem_timedwait(sem, &timeout_time);
    }

    if (status < 0)
    {
        if (errno == ETIMEDOUT)
        {
            /* If a time out occurred, return a timeout response */
            printf("Waiting for semaphore time out, time_out=%d ms\n", time_out);
        }
        else
        {
            /* If an error other than time out occurred, send an error response */
            printf("Waiting for semaphore failed, errno=%d(%m)\n", errno);
        }
    }

    return status;
}


/*==================================================================================================

    Module Name:  mbuf.c

    General Description: Implements the mbuf management

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include "mbuf.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief malloc mbuf poll

@param[in] count - how many mbuf the pool contains, each mbuf is a 2k bytes buffer

@return pointer to mbuf_pool
*//*==============================================================================================*/
mbuf_pool_t* mbuf_pool_alloc(unsigned count)
{
    ssize_t      real_size;
    int          mbuf_offset;
    mbuf_pool_t* p_mbuf_pool = malloc(sizeof(mbuf_pool_t));
    hp_t*        p_hp;


    if (p_mbuf_pool == NULL)
    {
        return NULL;
    }

    real_size   = rte_ring_get_memsize(count);
    real_size   = RTE_ALIGN(real_size, MBUF_SIZE);
    mbuf_offset = real_size;
    real_size  += MBUF_SIZE * count;

    p_hp = hp_alloc(real_size);

    if (p_hp == NULL)
    {
        printf("%s(): cannot alloc huge page\n", __func__);
        free(p_mbuf_pool);
        p_mbuf_pool = NULL;
    }
    else
    {
        int              i;
        struct rte_ring* r      = p_hp->base;
        void*            p_mbuf = p_hp->base + mbuf_offset;

        p_mbuf_pool->p_hp      = p_hp;
        p_mbuf_pool->p_ring    = r;
        p_mbuf_pool->mbuf_base = p_mbuf;

        rte_ring_init(p_hp->base, count);

        for (i = 0; i < count; i++)
        {
            r->ring[i] = p_mbuf;
            p_mbuf    += MBUF_SIZE;
        }

        /* till now the ring should be full */
        r->prod.head += count;
        r->prod.tail += count;

        if (!rte_ring_full(r))
        {
            printf("%s(): the mbuf pool ring is not full!\n", __func__);
            mbuf_pool_free(p_mbuf_pool);
            exit(1);
        }
    }

    return p_mbuf_pool;
}

/*=============================================================================================*//**
@brief free mbuf pool

@param[in] pool - pointer to mbuf pool
*//*==============================================================================================*/
void mbuf_pool_free(mbuf_pool_t* pool)
{
    if (pool == NULL)
    {
        return;
    }

    hp_free(pool->p_hp);
    free(pool);
}

/*=============================================================================================*//**
@brief get one mbuf

@param[in] pool - pointer to mbuf pool
*//*==============================================================================================*/
void* mbuf_get(mbuf_pool_t* pool)
{
    void* mbuf = NULL;

    /* this is sc dequeue for the rx thread, not mc */
    if (rte_ring_sc_dequeue(pool->p_ring, &mbuf) != 0)
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
void mbuf_put(mbuf_pool_t* pool, void* mbuf)
{
    if (rte_ring_mp_enqueue(pool->p_ring, &mbuf) != 0)
    {
        printf("%s(): can't enqueue mbuf to the mbuf pool!\n", __func__);
    }
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/


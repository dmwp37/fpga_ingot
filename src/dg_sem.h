#ifndef _DG_SEM_H_
#define _DG_SEM_H_

/**
 * @file
 *
 * RTE Read-Write Locks
 *
 * This file defines a busy wait semaphare. system sem_t is not efficient enough
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <errno.h>
#include "rte_common.h"
#include "rte_atomic.h"
#include "dg_dbg.h"

/**
 * The dg_sem_t type.
 */
typedef struct
{
    volatile int32_t cnt;
} dg_sem_t;

/**
 * A static semaphore initializer.
 */
#define DG_SEM_INITIALIZER { 0 }

/**
 * Initialize the semaphore.
 */
static inline void dg_sem_init(dg_sem_t* sem, uint32_t count)
{
    sem->cnt = count;
}

static inline int get_current_time()
{
    struct timeval timeofday;

    if (gettimeofday(&timeofday, NULL) != 0)
    {
        DG_DBG_ERROR("Failed to get time of day, errno=%d(%m)", errno);
        return -1;
    }

    return timeofday.tv_sec * 1000 + timeofday.tv_usec / 1000;
}

/**
 * semaphore wait.
 */
static inline int dg_sem_wait(dg_sem_t* sem, uint32_t time)
{
    uint32_t x;
    int      success  = 0;
    int      time_out = time + get_current_time();

    while (success == 0)
    {
        x = sem->cnt;
        if (x == 0)
        {
            if (unlikely(get_current_time() >= time_out))
            {
                DG_DBG_ERROR("dg_sem_wait timeout, time=%d ms", time);
                return -1;
            }
            /* rte_pause(); */
            /* pthread_yield(); */
            continue;
        }
        success = rte_atomic32_cmpset((volatile uint32_t*)&sem->cnt, x, x - 1);
    }
    return 0;
}

/**
 * semaphore post.
 */
static inline void dg_sem_post(dg_sem_t* sem)
{
    rte_atomic32_inc((rte_atomic32_t*)(intptr_t)&sem->cnt);
}

#ifdef __cplusplus
}
#endif

#endif /* _DG_SEM_H_ */


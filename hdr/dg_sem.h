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
/* pthread would cause the wait thread slowly recover and cause time out */
#undef USE_PTHREAD

#include <sys/time.h>
#ifdef USE_PTHREAD
#include <pthread.h>
#endif
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
#ifdef USE_PTHREAD
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
#endif
} dg_sem_t;

/**
 * Initialize the semaphore.
 */
static inline void dg_sem_init(dg_sem_t* sem, uint32_t count)
{
    sem->cnt = count;
#ifdef USE_PTHREAD
    pthread_cond_init(&sem->cond, NULL);
    pthread_mutex_init(&sem->mutex, NULL);
#endif
}

#ifndef USE_PTHREAD
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
#endif

/**
 * semaphore wait.
 */
static inline int dg_sem_wait(dg_sem_t* sem, uint32_t time)
{
    uint32_t x;
    int      success = 0;

#ifdef USE_PTHREAD
    struct timespec time_out;
    struct timeval  now;

    gettimeofday(&now, NULL);
    time_out.tv_sec  = now.tv_sec;
    time_out.tv_nsec = (now.tv_usec + time * 1000) * 1000;
    time_out.tv_sec += time_out.tv_nsec / 1000000000;
    time_out.tv_nsec = time_out.tv_nsec % 1000000000;
#else
    int time_out = time + get_current_time();
#endif

    while (success == 0)
    {
        x = sem->cnt;
        if (x == 0)
        {
#ifdef USE_PTHREAD
            pthread_mutex_lock(&sem->mutex);
            if (pthread_cond_timedwait(&sem->cond, &sem->mutex, &time_out) == ETIMEDOUT)
            {
                /* timeout, do something */
                pthread_mutex_unlock(&sem->mutex);
                DG_DBG_ERROR("dg_sem_wait timeout, time=%d ms", time);
                return -1;
            }
            pthread_mutex_unlock(&sem->mutex);
#else
            if (unlikely(get_current_time() >= time_out))
            {
                DG_DBG_ERROR("dg_sem_wait timeout, time=%d ms", time);
                return -1;
            }
            /* rte_pause(); */
            /* pthread_yield(); */
#endif
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
#ifdef USE_PTHREAD
    pthread_cond_signal(&sem->cond);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* _DG_SEM_H_ */


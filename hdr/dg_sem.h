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

#define USE_PTHREAD

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

#ifdef USE_PTHREAD
    struct timespec time_out;

    while (1)
    {
        x = sem->cnt;
        if (x == 0)
        {
            /* use the wait method */
            rte_pause();
            continue;
        }

        if (likely(rte_atomic32_cmpset((volatile uint32_t*)&sem->cnt, x, x - 1)))
        {
            return 0;
        }
    }

    pthread_mutex_lock(&sem->mutex);
    clock_gettime(CLOCK_REALTIME, &time_out);
    time_out.tv_sec += time / 1000;
    time_out.tv_nsec = (time % 1000) * 1000000;
    time_out.tv_sec += time_out.tv_nsec / 1000000000;
    time_out.tv_nsec = time_out.tv_nsec % 1000000000;
    while (sem->cnt == 0)
    {
        if (pthread_cond_timedwait(&sem->cond, &sem->mutex, &time_out) == ETIMEDOUT)
        {
            /* timeout, do something */
            pthread_mutex_unlock(&sem->mutex);
            DG_DBG_ERROR("dg_sem_wait timeout, time=%d ms", time);
            return -1;
        }
    }
    sem->cnt -= 1;
    pthread_mutex_unlock(&sem->mutex);
#else
    int success  = 0;
    int time_out = time + get_current_time();

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
#endif
    return 0;
}

/**
 * semaphore post.
 */
static inline void dg_sem_post(dg_sem_t* sem)
{
#ifdef USE_PTHREAD
    pthread_mutex_lock(&sem->mutex);
    pthread_cond_signal(&sem->cond);
    /* sem->cnt += 1; */
    rte_atomic32_inc((rte_atomic32_t*)(intptr_t)&sem->cnt);
    pthread_mutex_unlock(&sem->mutex);
#else
    rte_atomic32_inc((rte_atomic32_t*)(intptr_t)&sem->cnt);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* _DG_SEM_H_ */


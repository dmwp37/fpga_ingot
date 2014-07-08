/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Derived from FreeBSD's bufring.h
 *
 **************************************************************************
 *
 * Copyright (c) 2007-2009 Kip Macy kmacy@freebsd.org
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. The name of Kip Macy nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************/

#ifndef _RTE_RING_H_
#define _RTE_RING_H_

/**
 * @file
 * RTE Ring
 *
 * The Ring Manager is a fixed-size queue, implemented as a table of
 * pointers. Head and tail pointers are modified atomically, allowing
 * concurrent access to it. It has the following features:
 *
 * - FIFO (First In First Out)
 * - Maximum size is fixed; the pointers are stored in a table.
 * - Lockless implementation.
 * - Multi- or single-consumer dequeue.
 * - Multi- or single-producer enqueue.
 * - Bulk dequeue.
 * - Bulk enqueue.
 *
 * Note: the ring implementation is not preemptable. A lcore must not
 * be interrupted by another task that uses the same ring.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/queue.h>
#include <errno.h>

#include "rte_common.h"
#include "rte_atomic.h"

    
enum rte_ring_queue_behavior {
	RTE_RING_QUEUE_FIXED = 0, /* Enq/Deq a fixed number of items from a ring */
	RTE_RING_QUEUE_VARIABLE   /* Enq/Deq as many items a possible from ring */
};

/**
 * An RTE ring structure.
 *
 * The producer and the consumer have a head and a tail index. The particularity
 * of these index is that they are not between 0 and size(ring). These indexes
 * are between 0 and 2^32, and we mask their value when we access the ring[]
 * field. Thanks to this assumption, we can do subtractions between 2 index
 * values in a modulo-32bit base: that's why the overflow of the indexes is not
 * a problem.
 */
struct rte_ring {
	/** Ring producer status. */
	struct prod {
		uint32_t watermark;      /**< Maximum items before EDQUOT. */
		uint32_t size;           /**< Size of ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Producer head. */
		volatile uint32_t tail;  /**< Producer tail. */
	} prod __rte_cache_aligned;

	/** Ring consumer status. */
	struct cons {
		uint32_t size;           /**< Size of the ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Consumer head. */
		volatile uint32_t tail;  /**< Consumer tail. */
	} cons;

	void * ring[0] __rte_cache_aligned; /**< Memory space of ring starts here.
	                                     * not volatile so need to be careful
	                                     * about compiler re-ordering */
};

#define RTE_RING_QUOT_EXCEED (1 << 31)  /**< Quota exceed for burst ops */
#define RTE_RING_SZ_MASK  (unsigned)(0x0fffffff) /**< Ring size mask */


/**
 * Calculate the memory size needed for a ring
 *
 * This function returns the number of bytes needed for a ring, given
 * the number of elements in it. This value is the sum of the size of
 * the structure rte_ring and the size of the memory needed by the
 * objects pointers. The value is aligned to a cache line size.
 *
 * @param count
 *   The number of elements in the ring (must be a power of 2).
 * @return
 *   - The memory size needed for the ring on success.
 *   - -EINVAL if count is not a power of 2.
 */
ssize_t rte_ring_get_memsize(unsigned count);

/**
 * Initialize a ring structure.
 *
 * Initialize a ring structure in memory pointed by "r". The size of the
 * memory area must be large enough to store the ring structure and the
 * object table. It is advised to use rte_ring_get_memsize() to get the
 * appropriate size.
 *
 * The ring size is set to *count*, which must be a power of two. Water
 * marking is disabled by default. The real usable ring size is
 * *count-1* instead of *count* to differentiate a free ring from an
 * empty ring.
 *
 * The ring is not added in RTE_TAILQ_RING global list. Indeed, the
 * memory given by the caller may not be shareable among dpdk
 * processes.
 *
 * @param r
 *   The pointer to the ring structure followed by the objects table.
 * @param name
 *   The name of the ring.
 * @param count
 *   The number of elements in the ring (must be a power of 2).
 * @return
 *   0 on success, or a negative value on error.
 */
int rte_ring_init(struct rte_ring *r, unsigned count);


/**
 * Change the high water mark.
 *
 * If *count* is 0, water marking is disabled. Otherwise, it is set to the
 * *count* value. The *count* value must be greater than 0 and less
 * than the ring size.
 *
 * This function can be called at any time (not necessarily at
 * initialization).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   The new water mark value.
 * @return
 *   - 0: Success; water mark changed.
 *   - -EINVAL: Invalid water mark value.
 */
int rte_ring_set_water_mark(struct rte_ring *r, unsigned count);

/* the actual enqueue of pointers on the ring.
 * Placed here since identical code needed in both
 * single and multi producer enqueue functions */
#define ENQUEUE_PTRS() do { \
	const uint32_t size = r->prod.size; \
	uint32_t idx = prod_head & mask; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) { \
			r->ring[idx] = obj_table[i]; \
			r->ring[idx+1] = obj_table[i+1]; \
			r->ring[idx+2] = obj_table[i+2]; \
			r->ring[idx+3] = obj_table[i+3]; \
		} \
		switch (n & 0x3) { \
			case 3: r->ring[idx++] = obj_table[i++]; \
			case 2: r->ring[idx++] = obj_table[i++]; \
			case 1: r->ring[idx++] = obj_table[i++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++)\
			r->ring[idx] = obj_table[i]; \
		for (idx = 0; i < n; i++, idx++) \
			r->ring[idx] = obj_table[i]; \
	} \
} while(0)

/* the actual copy of pointers on the ring to obj_table.
 * Placed here since identical code needed in both
 * single and multi consumer dequeue functions */
#define DEQUEUE_PTRS() do { \
	uint32_t idx = cons_head & mask; \
	const uint32_t size = r->cons.size; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & (~(unsigned)0x3)); i+=4, idx+=4) {\
			obj_table[i] = r->ring[idx]; \
			obj_table[i+1] = r->ring[idx+1]; \
			obj_table[i+2] = r->ring[idx+2]; \
			obj_table[i+3] = r->ring[idx+3]; \
		} \
		switch (n & 0x3) { \
			case 3: obj_table[i++] = r->ring[idx++]; \
			case 2: obj_table[i++] = r->ring[idx++]; \
			case 1: obj_table[i++] = r->ring[idx++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
		for (idx = 0; i < n; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
	} \
} while (0)

/**
 * @internal Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   RTE_RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   RTE_RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RTE_RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = RTE_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
__rte_ring_mp_do_enqueue(struct rte_ring *r, void * const *obj_table,
			 unsigned n, enum rte_ring_queue_behavior behavior)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail, free_entries;
	const unsigned max = n;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	/* move prod.head atomically */
	do {
		/* Reset n to the initial burst count */
		n = max;

		prod_head = r->prod.head;
		cons_tail = r->cons.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * prod_head > cons_tail). So 'free_entries' is always between 0
		 * and size(ring)-1. */
		free_entries = (mask + cons_tail - prod_head);

		/* check that we have enough room in ring */
		if (unlikely(n > free_entries)) {
			if (behavior == RTE_RING_QUEUE_FIXED) {
				return -ENOBUFS;
			}
			else {
				/* No free entry available */
				if (unlikely(free_entries == 0)) {
					return 0;
				}

				n = free_entries;
			}
		}

		prod_next = prod_head + n;
		success = rte_atomic32_cmpset(&r->prod.head, prod_head,
					      prod_next);
	} while (unlikely(success == 0));

	/* write entries in ring */
	ENQUEUE_PTRS();
	rte_compiler_barrier();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = (behavior == RTE_RING_QUEUE_FIXED) ? -EDQUOT :
				(int)(n | RTE_RING_QUOT_EXCEED);
	}
	else {
		ret = (behavior == RTE_RING_QUEUE_FIXED) ? 0 : n;
	}

	/*
	 * If there are other enqueues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->prod.tail != prod_head))
		rte_pause();

	r->prod.tail = prod_next;
	return ret;
}

/**
 * @internal Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   RTE_RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   RTE_RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RTE_RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = RTE_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
__rte_ring_sp_do_enqueue(struct rte_ring *r, void * const *obj_table,
			 unsigned n, enum rte_ring_queue_behavior behavior)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next, free_entries;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	prod_head = r->prod.head;
	cons_tail = r->cons.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * prod_head > cons_tail). So 'free_entries' is always between 0
	 * and size(ring)-1. */
	free_entries = mask + cons_tail - prod_head;

	/* check that we have enough room in ring */
	if (unlikely(n > free_entries)) {
		if (behavior == RTE_RING_QUEUE_FIXED) {
			return -ENOBUFS;
		}
		else {
			/* No free entry available */
			if (unlikely(free_entries == 0)) {
				return 0;
			}

			n = free_entries;
		}
	}

	prod_next = prod_head + n;
	r->prod.head = prod_next;

	/* write entries in ring */
	ENQUEUE_PTRS();
	rte_compiler_barrier();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = (behavior == RTE_RING_QUEUE_FIXED) ? -EDQUOT :
			(int)(n | RTE_RING_QUOT_EXCEED);
	}
	else {
		ret = (behavior == RTE_RING_QUEUE_FIXED) ? 0 : n;
	}

	r->prod.tail = prod_next;
	return ret;
}

/**
 * @internal Dequeue several objects from a ring (multi-consumers safe). When
 * the request objects are more than the available objects, only dequeue the
 * actual number of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   RTE_RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   RTE_RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RTE_RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = RTE_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */

static inline int __attribute__((always_inline))
__rte_ring_mc_do_dequeue(struct rte_ring *r, void **obj_table,
		 unsigned n, enum rte_ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	const unsigned max = n;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;

	/* move cons.head atomically */
	do {
		/* Restore n as it may change every loop */
		n = max;

		cons_head = r->cons.head;
		prod_tail = r->prod.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * cons_head > prod_tail). So 'entries' is always between 0
		 * and size(ring)-1. */
		entries = (prod_tail - cons_head);

		/* Set the actual entries for dequeue */
		if (n > entries) {
			if (behavior == RTE_RING_QUEUE_FIXED) {
				return -ENOENT;
			}
			else {
				if (unlikely(entries == 0)){
					return 0;
				}

				n = entries;
			}
		}

		cons_next = cons_head + n;
		success = rte_atomic32_cmpset(&r->cons.head, cons_head,
					      cons_next);
	} while (unlikely(success == 0));

	/* copy in table */
	DEQUEUE_PTRS();
	rte_compiler_barrier();

	/*
	 * If there are other dequeues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->cons.tail != cons_head))
		rte_pause();

	r->cons.tail = cons_next;

	return behavior == RTE_RING_QUEUE_FIXED ? 0 : n;
}

/**
 * @internal Dequeue several objects from a ring (NOT multi-consumers safe).
 * When the request objects are more than the available objects, only dequeue
 * the actual number of objects
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   RTE_RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   RTE_RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RTE_RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = RTE_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */
static inline int __attribute__((always_inline))
__rte_ring_sc_do_dequeue(struct rte_ring *r, void **obj_table,
		 unsigned n, enum rte_ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	unsigned i;
	uint32_t mask = r->prod.mask;

	cons_head = r->cons.head;
	prod_tail = r->prod.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * cons_head > prod_tail). So 'entries' is always between 0
	 * and size(ring)-1. */
	entries = prod_tail - cons_head;

	if (n > entries) {
		if (behavior == RTE_RING_QUEUE_FIXED) {
			return -ENOENT;
		}
		else {
			if (unlikely(entries == 0)){
				return 0;
			}

			n = entries;
		}
	}

	cons_next = cons_head + n;
	r->cons.head = cons_next;

	/* copy in table */
	DEQUEUE_PTRS();
	rte_compiler_barrier();

	r->cons.tail = cons_next;
	return behavior == RTE_RING_QUEUE_FIXED ? 0 : n;
}

/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_mp_enqueue_bulk(struct rte_ring *r, void * const *obj_table,
			 unsigned n)
{
	return __rte_ring_mp_do_enqueue(r, obj_table, n, RTE_RING_QUEUE_FIXED);
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_sp_enqueue_bulk(struct rte_ring *r, void * const *obj_table,
			 unsigned n)
{
	return __rte_ring_sp_do_enqueue(r, obj_table, n, RTE_RING_QUEUE_FIXED);
}


/**
 * Enqueue one object on a ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_mp_enqueue(struct rte_ring *r, void *obj)
{
	return rte_ring_mp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_sp_enqueue(struct rte_ring *r, void *obj)
{
	return rte_ring_sp_enqueue_bulk(r, &obj, 1);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
rte_ring_mc_dequeue_bulk(struct rte_ring *r, void **obj_table, unsigned n)
{
	return __rte_ring_mc_do_dequeue(r, obj_table, n, RTE_RING_QUEUE_FIXED);
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
rte_ring_sc_dequeue_bulk(struct rte_ring *r, void **obj_table, unsigned n)
{
	return __rte_ring_sc_do_dequeue(r, obj_table, n, RTE_RING_QUEUE_FIXED);
}

/**
 * Dequeue one object from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
rte_ring_mc_dequeue(struct rte_ring *r, void **obj_p)
{
	return rte_ring_mc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
rte_ring_sc_dequeue(struct rte_ring *r, void **obj_p)
{
	return rte_ring_sc_dequeue_bulk(r, obj_p, 1);
}


/**
 * Test if a ring is full.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is full.
 *   - 0: The ring is not full.
 */
static inline int
rte_ring_full(const struct rte_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return (((cons_tail - prod_tail - 1) & r->prod.mask) == 0);
}

/**
 * Test if a ring is empty.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is empty.
 *   - 0: The ring is not empty.
 */
static inline int
rte_ring_empty(const struct rte_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return !!(cons_tail == prod_tail);
}

/**
 * Return the number of entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of entries in the ring.
 */
static inline unsigned
rte_ring_count(const struct rte_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return ((prod_tail - cons_tail) & r->prod.mask);
}

/**
 * Return the number of free entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of free entries in the ring.
 */
static inline unsigned
rte_ring_free_count(const struct rte_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return ((cons_tail - prod_tail - 1) & r->prod.mask);
}

/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_mp_enqueue_burst(struct rte_ring *r, void * const *obj_table,
			 unsigned n)
{
	return __rte_ring_mp_do_enqueue(r, obj_table, n, RTE_RING_QUEUE_VARIABLE);
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
rte_ring_sp_enqueue_burst(struct rte_ring *r, void * const *obj_table,
			 unsigned n)
{
	return __rte_ring_sp_do_enqueue(r, obj_table, n, RTE_RING_QUEUE_VARIABLE);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe). When the request
 * objects are more than the available objects, only dequeue the actual number
 * of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - n: Actual number of objects dequeued, 0 if ring is empty
 */
static inline int __attribute__((always_inline))
rte_ring_mc_dequeue_burst(struct rte_ring *r, void **obj_table, unsigned n)
{
	return __rte_ring_mc_do_dequeue(r, obj_table, n, RTE_RING_QUEUE_VARIABLE);
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).When the
 * request objects are more than the available objects, only dequeue the
 * actual number of objects
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - n: Actual number of objects dequeued, 0 if ring is empty
 */
static inline int __attribute__((always_inline))
rte_ring_sc_dequeue_burst(struct rte_ring *r, void **obj_table, unsigned n)
{
	return __rte_ring_sc_do_dequeue(r, obj_table, n, RTE_RING_QUEUE_VARIABLE);
}

#ifdef __cplusplus
}
#endif

#endif /* _RTE_RING_H_ */

#ifndef __PQ_H__
#define __PQ_H__

#include "include/engine-core/types.h"

#define PRIORITY_QUEUE_SIZE 322  // 321 possible moves at most

/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for managing the move priority queue                                       */
/* ------------------------------------------------------------------------------------------------ */

/* priority queue struct */
typedef struct _maxpq_t {
    int size;
    int nr_elem;
    move_t array[PRIORITY_QUEUE_SIZE];
} maxpq_t;

/* initializes a given priority queue */
void initialize_maxpq(maxpq_t* pq);
/* inserts a given element into the priority queue */
void insert(maxpq_t* pq, move_t elem);
/* Returns true if priority queue is empty */
int is_empty(maxpq_t* pq);
/* returns the element with maximal key of the priority queue */
move_t pop_max(maxpq_t* pq);
/* prints a given priority queue */
void print_pq(maxpq_t* pq);
/* sorts a given priority queue */
void heap_sort(maxpq_t* pq);

/* ------------------------------------------------------------------------------------------------ */
/* helper functions for priority queue management                                                   */
/* ------------------------------------------------------------------------------------------------ */

/* sink operation of heap (pq) */
void sink(maxpq_t* pq, int k);
/* swim operation of heap (pq) */
void swim(maxpq_t* pq, int k);
/* swaps two elements in the heap (pq) */
void swap(maxpq_t* pq, int i, int k);

#endif

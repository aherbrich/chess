#ifndef __PQ_H__
#define __PQ_H__

#include "../include/types.h"

#define PRIORITY_QUEUE_SIZE 322     // 321 possible moves at most 
typedef struct _maxpq_t {
    int size;
    int nr_elem;
    move_t* array[PRIORITY_QUEUE_SIZE];
} maxpq_t;

//  PRIORITY QUEUE FUNCTIONS
void initialize_maxpq(maxpq_t* pq);
void print_pq(maxpq_t* pq);

void sink(maxpq_t* pq, int k);
void swim(maxpq_t* pq, int k);
void swap(maxpq_t* pq, int i, int k);

void insert(maxpq_t* pq, move_t* elem);
move_t* pop_max(maxpq_t* pq);

void free_pq(maxpq_t* pq);

void heap_sort(maxpq_t* pq);


#endif
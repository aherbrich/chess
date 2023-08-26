#include "include/engine-core/pq.h"

#include <stdio.h>

#include "include/engine-core/pq.h"

#include "include/engine-core/types.h"
#include "include/engine-core/move.h"
#include "include/engine-core/prettyprint.h"

/* Returns true if value (of move) at index i is truely lower than value at
 * index j in queue */
int less(maxpq_t* pq, int i, int j) {
    return (pq->array[i]->value < pq->array[j]->value);
}

/* Swaps two elements (moves) in queue at indices i and j */
void swap(maxpq_t* pq, int i, int j) {
    move_t* tmp = pq->array[i];
    pq->array[i] = pq->array[j];
    pq->array[j] = tmp;
}

/* Initializes priority queue at given address */
void initialize_maxpq(maxpq_t* pq) {
    pq->size = PRIORITY_QUEUE_SIZE - 1;
    pq->nr_elem = 0;
    for (int i = 0; i < pq->size + 1; i++) {
        pq->array[i] = NULL;
    }
}

/* Prints priority queue (!in memory order, not logical order!) */
void print_pq(maxpq_t* pq) {
    for (int i = 1; i < pq->nr_elem + 1; i++) {
        print_move(pq->array[i]);
        fprintf(stderr, " (%d)", pq->array[i]->value);
        fprintf(stderr, "   ");
    }
    fprintf(stderr, "\n");
}

/* Swaps element at (initially) index k with its parent until heap property
 * satisfied */
void swim(maxpq_t* pq, int k) {
    while (k > 1 && less(pq, k / 2, k)) {
        swap(pq, k / 2, k);
        k = k / 2;
    }
}

/* Swaps element at (initially) index k with a child until heap property
 * satisfied */
void sink(maxpq_t* pq, int k) {
    while (2 * k <= pq->nr_elem) {
        /* index of first child */
        int j = 2 * k;
        /* set j to index of child with higher value */
        if (j < pq->nr_elem && less(pq, j, j + 1)) j++;
        /* if k'th element has higher value than both children we can exit early
         */
        if (!less(pq, k, j)) break;
        /* else swap */
        swap(pq, k, j);
        k = j;
    }
}

/* Sink helper function for heap sort */
void sink_N(maxpq_t* pq, int k, int N) {
    while (2 * k <= N) {
        /* index of first child */
        int j = 2 * k;
        /* set j to index of child with higher value */
        if (j < N && less(pq, j, j + 1)) j++;
        /* if k'th element has higher value than both children we can exit early
         */
        if (!less(pq, k, j)) break;
        /* else swap */
        swap(pq, k, j);
        k = j;
    }
}

/* Inserts element into priority queue while heap property stays satisfied */
void insert(maxpq_t* pq, move_t* elem) {
    pq->nr_elem++;
    if (pq->nr_elem > pq->size) {
        fprintf(stderr, "Out of space!\n");
        exit(1);
    }

    pq->array[pq->nr_elem] = elem;
    swim(pq, pq->nr_elem);
}

/* Removes element with highest value from queue and restores heap property */
move_t* pop_max(maxpq_t* pq) {
    /* if no elements in array return NULL to indicate empty array */
    if (pq->nr_elem == 0) {
        return NULL;
    }

    /* else */
    move_t* max = pq->array[1];
    pq->array[1] = pq->array[pq->nr_elem];
    pq->array[pq->nr_elem] = NULL;
    pq->nr_elem--;
    sink(pq, 1);
    return max;
}

/* Frees all moves in a given priority queue */
void free_pq(maxpq_t* pq) {
    /* we dont need to free pq explicitly since its memory was allocated on
     * stack */
    if (pq->nr_elem == 0) return;
    /* free moves */
    for (int i = 1; i < pq->nr_elem + 1; i++) {
        free_move(pq->array[i]);
    }
}

/* Heap sort */
void heap_sort(maxpq_t* pq) {
    int N = pq->nr_elem;
    for (int k = N / 2; k >= 1; k--) {
        sink_N(pq, k, N);
    }
    while (N > 1) {
        swap(pq, 1, N--);
        sink_N(pq, 1, N);
    }
}
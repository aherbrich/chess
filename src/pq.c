#include "../include/chess.h"
#include "../include/prettyprint.h"

int less(maxpq_t* pq, int i, int j){
    return((*pq).array[i]->value < (*pq).array[j]->value);
}

void swap(maxpq_t* pq, int i, int j){
    move_t* tmp = (*pq).array[i];
    (*pq).array[i] = (*pq).array[j];
    (*pq).array[j] = tmp; 
}

void initialize_maxpq(maxpq_t* pq){
    (*pq).size = sizeof((*pq).array)/sizeof(move_t*)-1;
    (*pq).nr_elem = 0;
    for(int i = 0; i < (*pq).size+1; i++){
        (*pq).array[i] = NULL;
    }
}

void print_pq(maxpq_t* pq){
    for(int i = 1; i < (*pq).nr_elem+1; i++){
        print_move((*pq).array[i]);
        fprintf(stderr, "   ");
    }
    printf("\n");
}

void swim(maxpq_t* pq, int k){
    while(k > 1 && less(pq, k/2, k)){
        swap(pq, k/2, k);
        k = k/2;
    }
}

void sink(maxpq_t* pq, int k){
    while(2*k <= (*pq).nr_elem){
        // index of first child
        int j = 2*k;
        // set j to index of child with higher value
        if(j < (*pq).nr_elem && less(pq, j, j+1)) j++;
        // if k'th element has higher value than both children we can exit early
        if(!less(pq, k, j)) break; 
        //else
        swap(pq, k, j);
        k = j;
    }
}

void insert(maxpq_t* pq, move_t* elem){
    (*pq).nr_elem++;
    if((*pq).nr_elem > (*pq).size){
        printf("Out of space!\n");
        exit(1);
    }

    (*pq).array[(*pq).nr_elem] = copy_move(elem);
    swim(pq, (*pq).nr_elem);
}

move_t* pop_max(maxpq_t* pq){
    /* if no elements in array return NULL to indicate empty array */
    if((*pq).nr_elem == 0){
        return NULL;
    }
    
    /* else */
    move_t* max = (*pq).array[1];
    (*pq).array[1] = (*pq).array[(*pq).nr_elem];
    (*pq).array[(*pq).nr_elem] = NULL;
    (*pq).nr_elem--;
    sink(pq, 1);
    return max;
}
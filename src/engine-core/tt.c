#include <stdio.h>
#include <string.h>

#include "include/engine-core/tt.h"

#include "include/engine-core/types.h"
#include "include/engine-core/move.h"
#include "include/engine-core/prettyprint.h"


/* ------------------------------------------------------------------------------------------------ */
/* helper functions for transposition table                                                         */
/* ------------------------------------------------------------------------------------------------ */

/* from 'Hackers solution' a fast way to calculate the nearest power of 2, lower or equal to x */
uint32_t round_to_power_of_two (uint32_t x) {
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    return x - (x >> 1);
}

/* finds the power of two */
int find_power_of_two(int x) {
    int count = 0;
    while(x >> count != 1) count++;
    return count;
}

/* fibonacci hash function for transposition table */
uint64_t hash_func_tt(uint64_t key, int bits) {
    return (key * 11400714819323198485ULL) >> (64-bits);
}

/* ------------------------------------------------------------------------------------------------ */
/* functions for intialization and deletion of transposition table                                  */
/* ------------------------------------------------------------------------------------------------ */


/* allocates memory for and initializes a transposition table */
tt_t init_tt(int size_in_bytes) {
    /* check if size is too small */
    if(size_in_bytes < (int) (sizeof(tt_bucket_t))){
        fprintf(stderr, "ERROR: transposition table size is too small\n");
        exit(EXIT_FAILURE);
    }

    /* calculate number of buckets */
    int nr_of_buckets =
        size_in_bytes /
        (sizeof(
            tt_bucket_t)); /* memory for every bucket (two entry pointers) */

    /* round (down) nr of buckets to power of two */
    nr_of_buckets = round_to_power_of_two(nr_of_buckets);

    /* allocate memory for transposition table */
    tt_t table;
    table.size = nr_of_buckets;
    table.no_bits = find_power_of_two(table.size);
    table.buckets = (tt_bucket_t*) malloc(nr_of_buckets * sizeof(tt_bucket_t));

    /* initialize every bucket */
    memset(table.buckets, 0, nr_of_buckets * sizeof(tt_bucket_t));

    return table;
}

/* frees memory of transposition table */
void free_tt(tt_t table) {
    free(table.buckets);
}

/* resets the transposition table */
void reset_tt(tt_t table) {
    memset(table.buckets, 0, table.size * sizeof(tt_bucket_t));
}


/* ------------------------------------------------------------------------------------------------ */
/* functions for storing and retrieving of transposition table entries                              */
/* ------------------------------------------------------------------------------------------------ */


/* stores an entry in transposition table */
void store_tt_entry(tt_t table, board_t* board, move_t move, int8_t depth, int16_t eval, int8_t flags) {
    /* calculate hash */
    uint64_t hash = hash_func_tt(board->hash, table.no_bits);

    /* get bucket */
    tt_bucket_t* bucket = &table.buckets[hash];
    
    /* get the pointer to the replace_always */
    tt_entry_t* entry_always_replace = &bucket->always_replace;

    /* fill fields of entry */
    entry_always_replace->key = board->hash;
    entry_always_replace->best_move = move;
    entry_always_replace->depth = depth;
    entry_always_replace->eval = eval;
    entry_always_replace->flags = flags;

    /* get the pointer to the replace_if_better */
    tt_entry_t* entry_if_better = &bucket->replace_if_better;

    if(entry_always_replace->depth > entry_if_better->depth) {
        /* fill fields of entry */
        entry_if_better->key = board->hash;
        entry_if_better->best_move = move;
        entry_if_better->depth = depth;
        entry_if_better->eval = eval;
        entry_if_better->flags = flags;
    }
}

/* retrieves an entry from transposition table */
tt_entry_t* retrieve_tt_entry(tt_t table, board_t* board) {
    /* calculate zobrist key and hash */
    uint64_t key = board->hash;
    uint64_t hash = hash_func_tt(key, table.no_bits);

    /* get bucket */
    tt_entry_t* entry_always_replace = &table.buckets[hash].always_replace;
    tt_entry_t* entry_replace_if_better = &table.buckets[hash].replace_if_better;

    if(entry_replace_if_better->key == key){
        return entry_replace_if_better;
    } else if (entry_always_replace->key == key){
        return entry_always_replace;
    }

    return NULL;
}

/* returns the eval for the board position based on tt entry */
int tt_eval(tt_t table, board_t* board) {
    /* calculate zobrist key and hash */
    uint64_t key = board->hash;
    uint64_t hash = hash_func_tt(board->hash, table.no_bits);

    /* get entries */
    tt_entry_t* entry_always_replace = &table.buckets[hash].always_replace;
    tt_entry_t* entry_replace_if_better = &table.buckets[hash].replace_if_better;

    /* return eval if there exist an entry for the board */
    if(entry_replace_if_better->key == key){
        return entry_replace_if_better->eval;
    } else if (entry_always_replace->key == key){
        return entry_always_replace->eval;
    }

    /* otherwise, return worst eval */
    return -16000;
}

/* Gets the best move for the board position based on tt entry */
move_t* tt_best_move(tt_t table, board_t *board) {
    /* calculate zobrist key and hash */
    uint64_t key = board->hash;
    uint64_t hash = hash_func_tt(board->hash, table.no_bits);

    /* get entries */
    tt_entry_t* entry_always_replace = &table.buckets[hash].always_replace;
    tt_entry_t* entry_replace_if_better = &table.buckets[hash].replace_if_better;

    /* return eval if there exist an entry for the board */
    if(entry_replace_if_better->key == key){
        return copy_move(&entry_replace_if_better->best_move);
    } else if (entry_always_replace->key == key){
        return copy_move(&entry_always_replace->best_move);
    }
    
    /* otherwise, return worst eval */
    return NULL;
}


/* ------------------------------------------------------------------------------------------------ */
/* functions for printing/info of transposition table                                               */
/* ------------------------------------------------------------------------------------------------ */


/* prints tt entry */
void print_tt_entry(tt_entry_t* entry) {
    printf("key: %llu\n", entry->key);
    printf("best move: ");
    print_move(entry->best_move);
    printf("depth: %d\n", entry->depth);
    printf("eval: %d\n", entry->eval);
    printf("flags: %d\n", entry->flags);
}

/* returns how full the transposition table is in per mille */
int tt_permille_full(tt_t table) {
    uint64_t count = 0;
    for (int i = 0; i < table.size; i++) {
        if (table.buckets[i].always_replace.key != 0ULL) count++;
        if (table.buckets[i].replace_if_better.key != 0ULL) count++;
    }
    return ((int)((count * 1000) / (table.size * 2)));
}

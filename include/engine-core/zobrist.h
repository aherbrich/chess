#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for zobrist hashing                                                        */
/* ------------------------------------------------------------------------------------------------ */

/* zobrist struct which holds 64-bit random numbers */
typedef struct _zobrist_t {
    uint64_t piece_random64[14][64];            /* random number for every piece-field pair*/
    uint64_t flag_random64[26];                 /* random number for every board flag */
} zobrist_t;

/* global zobrist table */
extern zobrist_t zobrist_table;             

/* initializes the global zobrist table */
void initialize_zobrist_table();
/* calculates zobrist hash for a given board */
uint64_t calculate_zobrist_hash(board_t *board);


/* ------------------------------------------------------------------------------------------------ */
/* defines, structs and functions for managing transposition table                                  */
/* ------------------------------------------------------------------------------------------------ */

// prime number should help avoid hash collisions
// #define HTSIZE  83471       // ~16MB
// #define HTSIZE  166703      // ~32MB
#define HTSIZE 334021  // ~64MB
// #define HTSIZE  18815231    // ~3.5GB

/* represents a single entry in the transposition table */
typedef struct _htentry_t {
    uint64_t hash;
    move_t *best_move;
    struct _htentry_t *next;
    int16_t eval;
    int8_t flags;
    int8_t depth;
} htentry_t;

/* global transposition table */
extern htentry_t **ht_table;

/* initializes the global transposition table */
void initialize_hashtable();
/* deletes (frees memory) of all transpositon table entries */
void reset_hashtable();
/* stores an entry in transposition table */
void store_hashtable_entry(board_t *board, int8_t flags, int16_t value, move_t *move, int8_t depth);
/* gets entry of TT, i.e. writes information into memory at given pointers, returns 1 if successfull */
int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth);
/* returns a copy of best move for a given board, returns NULL if there exist no such entry */
move_t *get_best_move_from_hashtable(board_t *board);
/* returns eval of best move for a given board, returns worst possible eval if there exist no such entry */
int get_eval_from_hashtable(board_t *board);
/* calculates number of bytes allocated for transposition table */
uint64_t get_memory_usage_hashtable_in_bytes();
/* calculates how full the transpostion table is in permill */
int hashtable_full_permill();

#endif
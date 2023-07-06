#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "../include/chess.h"

// prime number should help avoid hash collisions
//#define HTSIZE  83471       // ~16MB
//#define HTSIZE  166703      // ~32MB   
#define HTSIZE  334021      // ~64MB
//#define HTSIZE  18815231    // ~3.5GB  

typedef struct _zobrist_t {
    uint64_t piece_random64[12][64];
    uint64_t flag_random64[14];
} zobrist_t;

typedef struct _htentry_t {
    uint64_t hash;
    move_t* best_move;
    struct _htentry_t *next;
    int16_t eval;
    int8_t flags;
    int8_t depth;
} htentry_t;

//////////////////////////////////////////////////////////
//  GLOBALS
extern zobrist_t zobrist_table;
extern htentry_t** ht_table;

//////////////////////////////////////////////////////////
//  ZOBRIST HASHING
void initialize_zobrist_table();
uint64_t calculate_zobrist_hash(board_t *board);

//////////////////////////////////////////////////////////
//  TRANSPOSITION TABLE
void initialize_hashtable();
void clear_hashtable();
void store_hashtable_entry(board_t *board, int8_t flags, int16_t value, move_t *move, int8_t depth);
int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth);
move_t *get_best_move_from_hashtable(board_t* board);
int get_eval_from_hashtable(board_t* board);

uint64_t get_memory_usage_hashtable_in_bytes();
int hashtable_full_permill();

#endif
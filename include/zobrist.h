#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "../include/chess.h"

#define HTSIZE  67867967 // prime number should help avoid hash collisions

typedef struct _zobrist_t {
    uint64_t piece_random64[12][64];
    uint64_t flag_random64[14];
} zobrist_t;

typedef struct _htentry_t {
    int8_t flags;
    int16_t eval;
    int8_t depth;
    move_t* best_move;
    board_t* board;
    uint64_t hash;
    struct _htentry_t *next;
} htentry_t;

extern zobrist_t zobrist_table;
extern htentry_t** ht_table;

///////////////////////////////////////////////////////////////
//  ZOBRIST HASHING

extern void initialize_zobrist_table();
extern uint64_t calculate_zobrist_hash(board_t *board);
extern void initialize_hashtable();
extern void clear_hashtable();
extern void store_hashtable_entry(board_t *board, int8_t flags, int16_t value, move_t *move, int8_t depth);
extern int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth);
extern move_t *get_best_move_from_hashtable(board_t* board);
extern void print_move_and_board_from_hashtable(board_t* board);

#endif
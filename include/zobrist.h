#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "chess.h"
#include <stdint.h>

#define HTSIZE 67108864

typedef struct _zobrist_t {
    uint64_t hash_value[8][8][12];
    uint64_t hash_flags[14];
} zobrist_t;

extern zobrist_t zob_table;

typedef struct _htentry_t {
    int8_t flags;
    int16_t eval;
    int8_t depth;
    move_t* best_move;
    uint64_t hash;
    struct _htentry_t *next;
} htentry_t;

extern htentry_t** ht_table;

/* Initializes the global zobrist table */
extern void init_zobrist();
/* Zobrist-hashes a board using the zobrist table */
extern uint64_t zobrist(board_t* board);
/* Initializes a global hashtable */
extern void init_hashtable();
/* Clears the hashtable */
extern void clear_hashtable();
/* Stores key value pair in hashtable */
extern void storeTableEntry(board_t* board, int8_t flags, int16_t value, move_t* move, int8_t depth);
/* Probes table entry from hashtable and returns 1, if the entry is found (otherwise 0) */
extern int probeTableEntry(board_t* board, int8_t* flags, int16_t* value, move_t** move, int8_t* depth);
/* Gets the best move from the hashtable for the board position (or NULL, if there is not one) */
extern move_t *get_best_move(board_t* board);
/* Gets the eval from the hashtable for the board position */
extern int get_eval(board_t* board);

#endif
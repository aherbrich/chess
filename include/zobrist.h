#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "chess.h"
#include <stdint.h>

#define HTSIZE 67108864

typedef struct _zobrist_t {
    uint64_t hashvalue[8][8][12];
    uint64_t hashflags[13];
} zobrist_t;

extern zobrist_t zobtable;

typedef struct _htentry_t {
    int8_t flags;
    int16_t eval;
    int8_t depth;
    move_t* bestmove;
    uint64_t hash;
} htentry_t;

extern htentry_t* httable;

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
/* Probes table entry from hashtable */
extern void probeTableEntry(board_t* board, int8_t* flags, int16_t* value, move_t** move, int8_t* depth);

#endif
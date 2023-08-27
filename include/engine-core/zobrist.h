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
void initialize_zobrist_table(void);
/* calculates zobrist hash for a given board */
uint64_t calculate_zobrist_hash(board_t *board);

#endif

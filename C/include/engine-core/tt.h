#ifndef __TT_H__
#define __TT_H__


#include "include/engine-core/types.h"

#define MB_TO_BYTES(x) (x * 1024 * 1024)
#define BYTES_TO_MB(x) (x / 1024 / 1024)

/* ------------------------------------------------------------------------------------------------ */
/* structs for transposition table                                                                  */
/* ------------------------------------------------------------------------------------------------ */

/* transposition table entry */
typedef struct _tt_entry_t {
    uint64_t key;
    move_t best_move;
    int8_t depth;
    int32_t eval;
    int8_t flags; 
} tt_entry_t;

/* transposition table bucket */
typedef struct tt_bucket_t {
    tt_entry_t always_replace;
    tt_entry_t replace_if_better;
} tt_bucket_t;

/* transposition table */
typedef struct tt_t {
    tt_bucket_t* buckets;
    int size;
    int no_bits;
} tt_t;

/* ------------------------------------------------------------------------------------------------ */
/* functions for intialization and deletion of transposition table                                  */
/* ------------------------------------------------------------------------------------------------ */

/* allocates memory for and initializes a transposition table */
tt_t init_tt(int size_in_bytes);
/* frees memory for a transposition table */
void free_tt(tt_t table);
/* resets the transposition table */
void reset_tt(tt_t table);


/* ------------------------------------------------------------------------------------------------ */
/* functions for storing and retrieving of transposition table entries                              */
/* ------------------------------------------------------------------------------------------------ */

/* stores an entry in transposition table */
void store_tt_entry(tt_t table, board_t* board, move_t move, int8_t depth, int32_t eval, int8_t flags);
/* retrieves an entry from transposition table */
tt_entry_t* retrieve_tt_entry(tt_t table, board_t* board);
/* Returns the eval for the board position from tt */
int tt_eval(tt_t table, board_t* board);
/* Gets the best move for the board position from tt */
move_t *tt_best_move(tt_t table, board_t *board);

/* ------------------------------------------------------------------------------------------------ */
/* functions for printing/info of transposition table                                               */
/* ------------------------------------------------------------------------------------------------ */

/* prints tt entry */
void print_tt_entry(tt_entry_t* entry);
/* returns how full the transposition table is in per mille */
int tt_permille_full(tt_t table);

#endif

#include "include/engine-core/types.h"

#define MB_TO_BYTES(x) (x * 1024 * 1024)
#define BYTES_TO_MB(x) (x / 1024 / 1024)

/* transposition table entry */
typedef struct _tt_entry_t {
    uint64_t key;
    move_t best_move;
    int8_t depth;
    int16_t eval;
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

extern tt_t tt;

/* prints tt entry */
void print_tt_entry(tt_entry_t* entry);
/* allocates memory for and initializes a transposition table */
tt_t init_tt(int size_in_bytes);
/* stores an entry in transposition table */
void store_tt_entry(tt_t table, board_t* board, move_t move, int8_t depth, int16_t eval, int8_t flags);
/* retrieves an entry from transposition table */
tt_entry_t* retrieve_tt_entry(tt_t table, board_t* board);
/* returns how full the transposition table is in per mille */
int tt_permille_full(tt_t table);
/* Returns the eval for the board position from tt */
int tt_eval(tt_t table, board_t* board);
/* Gets the best move for the board position from tt */
move_t *tt_best_move(tt_t table, board_t *board);
/* resets the transposition table */
void reset_tt(tt_t table);
#ifndef __PERFT_H__
#define __PERFT_H__

#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for perft tests                                                                        */
/* ------------------------------------------------------------------------------------------------ */

/* runs a perft search */
uint64_t perft(board_t* board, int depth);
/* runs a perft divide search */
uint64_t perft_divide(board_t* board, int depth);

#endif
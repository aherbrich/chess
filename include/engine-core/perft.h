#ifndef __PERFT_H__
#define __PERFT_H__

#include "include/engine-core/types.h"

///////////////////////////////////////////////////////////////
//  PERFT
uint64_t perft(board_t* board, int depth);
uint64_t perft_divide(board_t* board, int depth);

#endif
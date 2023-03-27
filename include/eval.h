
#ifndef __EVAL_H__
#define __EVAL_H__

#include "../include/chess.h"

/////////////////////////////////////////////////////////////
//   EVALUATION

extern int eval_end_of_game(board_t *board, int depth);
extern int eval_board(board_t *board);

#endif
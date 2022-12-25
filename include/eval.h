#ifndef __EVAL_H__
#define __EVAL_H__

#include "chess.h"

/* Counts the material and its value of a specific color */
extern int count_material(board_t* board, player_t color);
/* Simple evaluation fucntion for negamax */
extern int eval_board(board_t* board);
/* Simple evaluation function of an ended game for negamax*/
extern int eval_end_of_game(board_t* board, int depth);

#endif
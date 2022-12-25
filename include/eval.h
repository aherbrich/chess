#ifndef __EVAL_H__
#define __EVAL_H__

#include "chess.h"

/* Counts the material and its value of a specific color */
extern int countMaterial(board_t* board, player_t color);
/* Simple evaluation fucntion for negamax */
extern int evalBoard(board_t* board);
/* Simple evaluation function of an ended game for negamax*/
extern int evalEndOfGame(board_t* board, int depth);

#endif
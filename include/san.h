#ifndef __SAN_H__
#define __SAN_H__

#include "../include/chess.h"

#define AMBIG_BY_RANK -1
#define AMBIG_BY_FILE 1

/* Converts a string (short algebraic notation) to a move */
move_t* str_to_move(board_t* board, char* token);

#endif
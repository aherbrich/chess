#ifndef __BOARD_H__
#define __BOARD_H__

#include "../include/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the board structure                                                       */
/* ------------------------------------------------------------------------------------------------ */

/* allocates memory and initiliazes board by call to 'clear_board' */
board_t* init_board();
/* makes a deep copy of a board */
board_t* copy_board(board_t* board);
/* resets the board to an empty, white at turn, default flags, no previous moves state */
void clear_board(board_t* board);
/* frees the memory of a board */
void free_board(board_t* board);
/* loads a board position based given a fen-string */
void load_by_FEN(board_t* board, char* FEN);

#endif
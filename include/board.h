#ifndef __BOARD_H__
#define __BOARD_H__

#include "../include/types.h"

//  BOARD FUNCTIONS
board_t* init_board();
board_t* copy_board(board_t* board);
void clear_board(board_t* board);
void free_board(board_t* board);
void load_by_FEN(board_t* board, char* FEN);

#endif
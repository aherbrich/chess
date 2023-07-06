#ifndef __PRETTYPRINT_H__
#define __PRETTYPRINT_H__

#include "../include/chess.h"

//////////////////////////////////////////////////////////////
//  PRINT FUNCTIONS
void print_board(board_t* board);
void print_bitboard(bitboard_t board);
void print_move(move_t* move);
void print_LAN_move(move_t* move, player_t color_playing);
char* get_LAN_move(move_t* move, player_t color_playing);
void print_line(board_t* board, int depth);

#endif
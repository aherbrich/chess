#ifndef __PRETTYPRINT_H__
#define __PRETTYPRINT_H__


#include "../include/chess.h"

//////////////////////////////////////////////////////////////
//  PRINT FUNCTIONS

extern void print_board(board_t* board);
extern void print_bitboard(bitboard_t board);
extern void print_move_on_board(move_t *move);
extern void print_move(move_t* move);
extern void print_LAN_move(move_t* move, player_t color_playing);
extern void print_line(board_t* board, int depth);
extern void print_movelist(list_t* movelst);

#endif
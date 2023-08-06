#ifndef __PRETTYPRINT_H__
#define __PRETTYPRINT_H__

#include "../include/types.h"

#define Color_YELLOW "\033[0;33m"
#define Color_GREEN "\033[0;32m"
#define Color_RED "\033[0;31m"
#define Color_CYAN "\033[0;36m"
#define Color_PURPLE "\033[0;35m"
#define Color_WHITE "\033[0;37m"
#define Color_END "\033[0m"

//////////////////////////////////////////////////////////////
//  PRINT FUNCTIONS
void print_board(board_t* board);
void print_bitboard(bitboard_t board);
void print_move(move_t* move);
void print_move_ranking(board_t* board, move_t* move);
void print_move_test(board_t* board, move_t* move);
void print_LAN_move(move_t* move, player_t color_playing);
char* get_LAN_move(move_t* move, player_t color_playing);
void print_line(board_t* board, int depth);

#endif
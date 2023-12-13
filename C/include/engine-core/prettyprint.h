#ifndef __PRETTYPRINT_H__
#define __PRETTYPRINT_H__

#include "include/engine-core/types.h"
#include "include/engine-core/tt.h"

#define Color_YELLOW "\033[0;33m"
#define Color_GREEN "\033[0;32m"
#define Color_RED "\033[0;31m"
#define Color_CYAN "\033[0;36m"
#define Color_PURPLE "\033[0;35m"
#define Color_WHITE "\033[0;37m"
#define Color_END "\033[0m"


/* ------------------------------------------------------------------------------------------------ */
/* functions for printing of board(-like) structures                                                */
/* ------------------------------------------------------------------------------------------------ */

/* prints a given chessboard */
void print_board(board_t* board);
/* prints a given bitboard; useful for bitboard debugging */
void print_bitboard(bitboard_t board);

/* ------------------------------------------------------------------------------------------------ */
/* functions for printing of move(-like) structures                                                */
/* ------------------------------------------------------------------------------------------------ */

/* prints a given move */
void print_move(move_t move);
/* prints a given move in LANotation */
void print_LAN_move(move_t move, player_t color_playing);
/* returns a string of a given move in LANotation */
char* get_LAN_move(move_t move, player_t color_playing);
/* prints the principal variation, i.e. the sequence of moves the engine considers best */
void print_line(tt_t tt, board_t* board, int depth);

#endif

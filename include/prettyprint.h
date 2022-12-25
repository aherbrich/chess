#ifndef __PRETTY_PRINT_H__
#define __PRETTY_PRINT_H__

#include "chess.h"

/* Print a piece */
extern char print_piece(piece_t piece);
/* Print the board */
extern void print_board(board_t* board);
/* Print move*/
extern void print_move(move_t* move);
/* Print list of moves */
extern void print_moves(node_t* move_list);
extern void print_line(board_t* board, int depth);

#endif
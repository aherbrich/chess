#ifndef __PRETTY_PRINT_H__
#define __PRETTY_PRINT_H__

#include "chess.h"

/* Print a piece */
extern char printPiece(piece_t piece);
/* Print the board */
extern void printBoard(board_t* board);
/* Print move*/
extern void printMove(move_t* move);
/* Print list of moves */
extern void printMoves(node_t* movelst);
extern void printLine(board_t* board, int depth);

#endif
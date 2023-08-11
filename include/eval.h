
#ifndef __EVAL_H__
#define __EVAL_H__

#include "../include/types.h"

#define PAWNVALUE 100
#define KNIGHTVALUE 320
#define BISHOPVALUE 330
#define ROOKVALUE 500
#define QUEENVALUE 900

extern const int MATERIAL_VALUE[16];

/* pawn position values */
extern int PAWN_POSITION_VALUE[64];
/* knight position values */
extern int KNIGHT_POSITION_VALUE[64];
/* bishop position values */
extern int BISHOP_POSITION_VALUE[64];
/* rook position values */
extern int ROOK_POSITION_VALUE[64];
/* queen position values */
extern int QUEEN_POSITION_VALUE[64];
/* king position values */
extern int KING_POSITION_VALUE[64];

/////////////////////////////////////////////////////////////
//  EVALUATION
int nr_of_pieces(board_t *board);
int eval_end_of_game(board_t *board, int depth);
int eval_board(board_t *board);

#endif
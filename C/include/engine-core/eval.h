#ifndef __EVAL_H__
#define __EVAL_H__

#include "include/engine-core/types.h"

#define PIECE_COLOR(p) ((p)>>3)
#define FLIP(sq) ((sq)^56)

#define PAWNVALUE 100
#define KNIGHTVALUE 320
#define BISHOPVALUE 330
#define ROOKVALUE 500
#define QUEENVALUE 900

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
/* 1-to-1 mapping from pieces to their material value */
extern const int MATERIAL_VALUE[16];

/* ------------------------------------------------------------------------------------------------ */
/* functions for simple evaluation                                                       */
/* ------------------------------------------------------------------------------------------------ */

/* returns simple evaluation (material and positional difference) */
int eval_board(board_t *board);

#endif

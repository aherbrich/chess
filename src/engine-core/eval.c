#include "include/engine-core/eval.h"

#include "include/engine-core/move.h"
#include "include/engine-core/types.h"

/* pawn position values */
int PAWN_POSITION_VALUE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10, 5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0, 5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0};

/* knight position values */
int KNIGHT_POSITION_VALUE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 0, 0,
    0, -20, -40, -30, 0, 10, 15, 15, 10, 0, -30, -30, 5,
    15, 20, 20, 15, 5, -30, -30, 0, 15, 20, 20, 15, 0,
    -30, -30, 5, 10, 15, 15, 10, 5, -30, -40, -20, 0, 5,
    5, 0, -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};

/* bishop position values */
int BISHOP_POSITION_VALUE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0, 0, 0, 0,
    0, 0, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 5,
    5, 10, 10, 5, 5, -10, -10, 0, 10, 10, 10, 10, 0,
    -10, -10, 10, 10, 10, 10, 10, 10, -10, -10, 5, 0, 0,
    0, 0, 5, -10, -20, -10, -10, -10, -10, -10, -10, -20};

/* rook position values */
int ROOK_POSITION_VALUE[64] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10,
                               10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,
                               0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0,
                               -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0,
                               0, 0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0};

/* queen position values */
int QUEEN_POSITION_VALUE[64] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10,
                                10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,
                                0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0,
                                -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0,
                                0, 0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0};

/* king position values */
int KING_POSITION_VALUE[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20, 20, 0, 0,
    0, 0, 20, 20, 20, 30, 10, 0, 0, 10, 30, 20};

/* 1-to-1 mapping from pieces to their material value */
const int MATERIAL_VALUE[16] = {-PAWNVALUE, -KNIGHTVALUE, -BISHOPVALUE, -ROOKVALUE, -QUEENVALUE, 0, 0, 0,
                                PAWNVALUE, KNIGHTVALUE, BISHOPVALUE, ROOKVALUE, QUEENVALUE, 0, 0, 0};

/* ------------------------------------------------------------------------------------------------ */
/* functions for simple evaluation                                                       */
/* ------------------------------------------------------------------------------------------------ */

/* returns number of pieces on board */
int nr_of_pieces(board_t *board) {
    int counter = 0;

    for (int i = 0; i < 64; i++) {
        if (board->playingfield[i] != NO_PIECE) counter++;
    }

    return counter;
}

/* returns either mate or stalemate evaluation score */
int eval_end_of_game(board_t *board, int depth) {
    /* check for stalemate */
    int in_check = is_in_check(board);

    /* if check (mate) delivered */
    if (in_check) {
        return -16000 - depth;
    }
    /* if stalemate */
    else {
        return 0;
    }
}

/* returns simple evaluation (material and positional difference) */
int eval_board(board_t *board) {
    int material = 0;
    int positional = 0;

    for (int i = 0; i < 64; i++) {
        material += MATERIAL_VALUE[board->playingfield[i]];
        switch (board->playingfield[i]) {
            case B_PAWN:
                positional -= PAWN_POSITION_VALUE[i];
                break;
            case B_KNIGHT:
                positional -= KNIGHT_POSITION_VALUE[i];
                break;
            case B_BISHOP:
                positional -= BISHOP_POSITION_VALUE[i];
                break;
            case B_ROOK:
                positional -= ROOK_POSITION_VALUE[i];
                break;
            case B_QUEEN:
                positional -= QUEEN_POSITION_VALUE[i];
                break;
            case B_KING:
                positional -= KING_POSITION_VALUE[i];
                break;
            case W_PAWN:
                positional += PAWN_POSITION_VALUE[63 - i];
                break;
            case W_KNIGHT:
                positional += KNIGHT_POSITION_VALUE[63 - i];
                break;
            case W_BISHOP:
                positional += BISHOP_POSITION_VALUE[63 - i];
                break;
            case W_ROOK:
                positional += ROOK_POSITION_VALUE[63 - i];
                break;
            case W_QUEEN:
                positional += QUEEN_POSITION_VALUE[63 - i];
                break;
            case W_KING:
                positional += KING_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }

    int eval = material + positional;

    /* due to negamax we want both players to maximize */
    if (board->player == BLACK) {
        eval *= -1;
    }

    return eval;
}
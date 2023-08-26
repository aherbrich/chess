#include "include/train-eval/features.h"

#include "include/engine-core/eval.h"
#include "include/engine-core/types.h"
#include "include/train-eval/linalg.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for training of a linear regression evaluation model                                   */
/* ------------------------------------------------------------------------------------------------ */

/* returns (normalized) material difference of board */
double material_difference(board_t* board) {
    int material = 0;
    for (int i = 0; i < 64; i++) {
        material += MATERIAL_VALUE[board->playingfield[i]];
    }
    return ((double)material) / 800.0;
}

/* returns (normalized) positional difference of board */
double positional_difference(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
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
    return ((double)positional) / 300.0;
}

/* returns (normalized) positional difference of pawns only */
double positional_diff_pawn(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_PAWN:
                positional -= PAWN_POSITION_VALUE[i];
                break;
            case W_PAWN:
                positional += PAWN_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* returns (normalized) positional difference of knights only */
double positional_diff_knight(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_KNIGHT:
                positional -= KNIGHT_POSITION_VALUE[i];
                break;
            case W_KNIGHT:
                positional += KNIGHT_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* returns (normalized) positional difference of bishops only */
double positional_diff_bishop(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_BISHOP:
                positional -= BISHOP_POSITION_VALUE[i];
                break;
            case W_BISHOP:
                positional += BISHOP_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* returns (normalized) positional difference of rooks only */
double positional_diff_rook(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_ROOK:
                positional -= ROOK_POSITION_VALUE[i];
                break;
            case W_ROOK:
                positional += ROOK_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* returns (normalized) positional difference of queens only */
double positional_diff_queen(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_QUEEN:
                positional -= QUEEN_POSITION_VALUE[i];
                break;
            case W_QUEEN:
                positional += QUEEN_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* returns (normalized) positional difference of kings only */
double positional_diff_king(board_t* board) {
    int positional = 0;
    for (int i = 0; i < 64; i++) {
        switch (board->playingfield[i]) {
            case B_KING:
                positional -= KING_POSITION_VALUE[i];
                break;
            case W_KING:
                positional += KING_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double)positional) / 30.0;
}

/* calculates feature matrix */
void calculate_feautures(board_t* board, matrix_t* X, int idx) {
}
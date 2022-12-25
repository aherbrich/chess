#include "../include/chess.h"

//////////////////////////////////////////////////////////////
//  USEFUL FUNCTIONS

/* Calculate idx based on a row & column */
idx_t pos_to_idx(int row, int col) {
    return (row * 8 + col);
}

/* Copies flags from board */
oldflags_t* copy_flags(board_t* board) {
    oldflags_t* oldflags = (oldflags_t*)malloc(sizeof(oldflags_t));

    oldflags->castle_rights = board->castle_rights;
    oldflags->ep_possible = board->ep_possible;
    oldflags->ep_field = board->ep_field;

    return oldflags;
}

/* Copies flags from move */
oldflags_t* copy_flags_from_move(move_t* move) {
    oldflags_t* oldflagsfrommove = (oldflags_t*)malloc(sizeof(oldflags_t));

    oldflagsfrommove->castle_rights = move->oldflags->castle_rights;
    oldflagsfrommove->ep_possible = move->oldflags->ep_possible;
    oldflagsfrommove->ep_field = move->oldflags->ep_field;

    return oldflagsfrommove;
}

/* Max of function */
int max(int x, int y) {
    if (x > y) {
        return x;
    }
    return y;
}

/* Min of function */
int min(int x, int y) {
    if (x < y) {
        return x;
    }
    return y;
}

#include "../include/chess.h"

//////////////////////////////////////////////////////////////
//  USEFUL FUNCTIONS

/* Calculate idx based on a row & column */
idx_t posToIdx(int row, int col) {
    return (row * 8 + col);
}

/* Copies flags from board */
oldflags_t* copyflags(board_t* board) {
    oldflags_t* oldflags = (oldflags_t*)malloc(sizeof(oldflags_t));

    oldflags->castle_rights = board->castle_rights;
    oldflags->ep_possible = board->ep_possible;
    oldflags->ep_field = board->ep_field;

    return oldflags;
}

/* Copies flags from move */
oldflags_t* copyflagsfrommove(move_t* move) {
    oldflags_t* oldflagsfrommove = (oldflags_t*)malloc(sizeof(oldflags_t));

    oldflagsfrommove->castle_rights = move->oldflags->castle_rights;
    oldflagsfrommove->ep_possible = move->oldflags->ep_possible;
    oldflagsfrommove->ep_field = move->oldflags->ep_field;

    return oldflagsfrommove;
}

/* Max of function */
int maxof(int x, int y) {
    if (x > y) {
        return x;
    }
    return y;
}

/* Min of function */
int minof(int x, int y) {
    if (x < y) {
        return x;
    }
    return y;
}

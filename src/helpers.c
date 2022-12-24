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

    oldflags->castlerights = board->castlerights;
    oldflags->eppossible = board->eppossible;
    oldflags->epfield = board->epfield;

    return oldflags;
}

/* Copies flags from move */
oldflags_t* copyflagsfrommove(move_t* move) {
    oldflags_t* oldflagsfrommove = (oldflags_t*)malloc(sizeof(oldflags_t));

    oldflagsfrommove->castlerights = move->oldflags->castlerights;
    oldflagsfrommove->eppossible = move->oldflags->eppossible;
    oldflagsfrommove->epfield = move->oldflags->epfield;

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

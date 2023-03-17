#include "../include/chess.h"
#include <stdio.h>
#include <assert.h>

/* Calculate idx based on a row & column */
idx_t pos_to_idx(int row, int col) {
    return (row * 8 + col);
}

/* Returns index of least significant bit */
int find_1st_bit(bitboard_t bb){
    const int lsb_64_table[64] = {
        63, 30,  3, 32, 59, 14, 11, 33,
        60, 24, 50,  9, 55, 19, 21, 34,
        61, 29,  2, 53, 51, 23, 41, 18,
        56, 28,  1, 43, 46, 27,  0, 35,
        62, 31, 58,  4,  5, 49, 54,  6,
        15, 52, 12, 40,  7, 42, 45, 16,
        25, 57, 48, 13, 10, 39,  8, 44,
        20, 47, 38, 22, 17, 37, 36, 26
        };

    assert(bb);
    unsigned int folded;
    assert (bb != 0);
    bb ^= bb - 1;
    folded = (int) bb ^ (bb >> 32);
    return lsb_64_table[folded * 0x78291ACF >> 26];
}

/* Sets the LS1B (least significant 1 bit) to 0 and returns it index */
int pop_1st_bit(bitboard_t *bitboard){
    if(!(*bitboard)){
        return -1;
    }

    int idx = find_1st_bit(*bitboard);
    *bitboard &= *bitboard - 1;
    return idx;
}

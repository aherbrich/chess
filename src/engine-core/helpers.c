#include "../../include/types.h"
#include "../../include/move.h"
#include "../../include/zobrist.h"

const int DEBRUIJN64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

const bitboard_t MAGICDEBRUIJN = 0x03f79d71b4cb0a89;

/* Generates a pseudo-random 64bit unsigned integer */
uint64_t random_uint64() {
    uint64_t u1, u2, u3, u4;
    u1 = (uint64_t)(rand()) & 0xFFFF;
    u2 = (uint64_t)(rand()) & 0xFFFF;
    u3 = (uint64_t)(rand()) & 0xFFFF;
    u4 = (uint64_t)(rand()) & 0xFFFF;
    return (u1 | (u2 << 16) | (u3 << 32) | (u4 << 48));
}

/* Generates a pseudo-random 64bit unsigned integer with only few bits set */
uint64_t random_uint64_fewbits() {
    return random_uint64() & random_uint64() & random_uint64();
}

/* Calculate idx based on a row & column */
idx_t pos_to_idx(int row, int col) { return (row * 8 + col); }

/* Returns index of least significant bit */
int find_1st_bit(bitboard_t bb) {
    return DEBRUIJN64[MAGICDEBRUIJN * (bb ^ (bb - 1)) >> 58];
}

/* Sets the LS1B (least significant 1 bit) to 0 and returns it index */
int pop_1st_bit(bitboard_t *bb) {
    if (!(*bb)) {
        return -1;
    }

    int idx = find_1st_bit(*bb);
    *bb &= *bb - 1;
    return idx;
}

/* Counts number of set bits in bitboard */
int sparse_pop_count(bitboard_t x) {
	int count = 0;
	/* while 64-bit number != 0 */
	while (x) {
		count++;
		/* set ls1b to zero and continue counting */
		x &= x - 1;
	}
	return count;
}

rank_t rank_of(square_t s) { return s >> 3; }
file_t file_of(square_t s) { return s & 0b111; }
int diagonal_of(square_t s) { return 7 + rank_of(s) - file_of(s); }
int anti_diagonal_of(square_t s) { return rank_of(s) + file_of(s); }

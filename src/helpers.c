#include "../include/chess.h"
#include "../include/magic.h"

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

int rank_of(square_t s) { return s >> 3; }
int file_of(square_t s) { return s & 0b111; }
int diagonal_of(square_t s) { return 7 + rank_of(s) - file_of(s); }
int anti_diagonal_of(square_t s) { return rank_of(s) + file_of(s); }

bitboard_t reverse(bitboard_t b) {
	b = ((b & 0x5555555555555555) << 1) | ((b >> 1) & 0x5555555555555555);
	b = ((b & 0x3333333333333333) << 2) | ((b >> 2) & 0x3333333333333333);
	b = ((b & 0x0f0f0f0f0f0f0f0f) << 4) | ((b >> 4) & 0x0f0f0f0f0f0f0f0f);
	b = ((b & 0x00ff00ff00ff00ff) << 8) | ((b >> 8) & 0x00ff00ff00ff00ff);

	return (b << 48) | ((b & 0xffff0000) << 16) |
		((b >> 16) & 0xffff0000) | (b >> 48);
}

bitboard_t sliding_attacks(square_t square, bitboard_t occ, bitboard_t mask) {
	return (((mask & occ) - SQUARE_BB[square] * 2) ^
		reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask;
}

bitboard_t get_rook_attacks_for_init(square_t square, bitboard_t occ) {
	return sliding_attacks(square, occ, MASK_FILE[file_of(square)]) |
		sliding_attacks(square, occ, MASK_RANK[rank_of(square)]);
}

bitboard_t get_bishop_attacks_for_init(square_t square, bitboard_t occ) {
	return sliding_attacks(square, occ, MASK_DIAGONAL[diagonal_of(square)]) |
		sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[anti_diagonal_of(square)]);
}

void initialize_squares_between() {
	bitboard_t sqs;
	for (square_t sq1 = a1; sq1 <= h8; ++sq1)
		for (square_t sq2 = a1; sq2 <= h8; ++sq2) {
			sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
			if (file_of(sq1) == file_of(sq2) || rank_of(sq1) == rank_of(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_bishop_attacks_for_init(sq1, sqs) & get_bishop_attacks_for_init(sq2, sqs);
		}
}

void initialize_line() {
	for (square_t sq1 = a1; sq1 <= h8; ++sq1)
		for (square_t sq2 = a1; sq2 <= h8; ++sq2) {
			if (file_of(sq1) == file_of(sq2) || rank_of(sq1) == rank_of(sq2))
				LINE[sq1][sq2] =
				(get_rook_attacks_for_init(sq1, 0ULL) & get_rook_attacks_for_init(sq2, 0ULL))
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				LINE[sq1][sq2] =
				(get_bishop_attacks_for_init(sq1, 0ULL) & get_bishop_attacks_for_init(sq2, 0ULL))
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
		}
}

/* Initializes hashtables with attack board for each square */
void initialize_attack_boards() {
    /* bishop attacks */
    for (int sq = 0; sq < 64; sq++) {
        bitboard_t mask = bishop_mask(sq);
        BISHOP_ATTACK_MASK[sq] = mask;
        
        /* pre-generate attack boards for each square and blocking mask with */
        /* magic bitboards */
        for (int i = 0; i < (1 << BISHOP_BITS[sq]); i++) {
            bitboard_t blockermap = index_to_bitboard(i, BISHOP_BITS[sq], mask);
            int j = transform(blockermap, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
            BISHOP_ATTACK[sq][j] = bishop_attacks(sq, blockermap);
        }
    }
    /* rook attacks */
    for (int sq = 0; sq < 64; sq++) {
        bitboard_t mask = rook_mask(sq);
        ROOK_ATTACK_MASK[sq] = mask;

        /* pre-generate attack boards for each square and blocking mask with
         * magic bitboards */
        for (int i = 0; i < (1 << ROOK_BITS[sq]); i++) {
            bitboard_t blockermap = index_to_bitboard(i, ROOK_BITS[sq], mask);
            int j = transform(blockermap, ROOK_MAGIC[sq], ROOK_BITS[sq]);
            ROOK_ATTACK[sq][j] = rook_attacks(sq, blockermap);
        }
    }
    /* knight attacks */
    for (int sq = 0; sq < 64; sq++) {
        /* pre-generate attack boards for each square */
        bitboard_t knight = (1ULL << sq);
        bitboard_t attacks =
            ((knight & CLEAR_FILE[A] & CLEAR_FILE[B]) << 6 |
             (knight & CLEAR_FILE[A] & CLEAR_FILE[B]) >> 10) |
            ((knight & CLEAR_FILE[G] & CLEAR_FILE[H]) << 10 |
             (knight & CLEAR_FILE[G] & CLEAR_FILE[H]) >> 6) |
            ((knight & CLEAR_FILE[A]) << 15 | (knight & CLEAR_FILE[A]) >> 17) |
            ((knight & CLEAR_FILE[H]) << 17 | (knight & CLEAR_FILE[H]) >> 15);
        KNIGHT_ATTACK[sq] = attacks;
    }
    /* king attacks */
    for (int sq = 0; sq < 64; sq++) {
        /* pre-generate attack boards for each square */
        bitboard_t king = (1ULL << sq);
        bitboard_t attacks =
            ((king & CLEAR_FILE[A]) >> 1 | (king & CLEAR_FILE[A]) >> 9 |
             (king & CLEAR_FILE[A]) << 7) |
            ((king & CLEAR_FILE[H]) << 1 | (king & CLEAR_FILE[H]) << 9 |
             (king & CLEAR_FILE[H]) >> 7) |
            ((king >> 8) | (king << 8));
        KING_ATTACK[sq] = attacks;
    }
}

/* Initializes hashtables with masks for ranks and files */
/* WARNING: Should always be called BEFORE initialize_attack_boards */
void initialize_helper_boards() {
    initialize_squares_between();
    initialize_line();
}

/* Initializes old state array */
void initialize_oldstate_array() {
    for (int i = 0; i < 2048; i++) {
        OLDSTATE[i] = 0;
    }
}

/* Initializes old state array */
void initialize_history_hash_array() {
    for (int i = 0; i < 2048; i++) {
        HISTORY_HASHES[i] = 0;
    }
}

/* Initializes all structures necessary for legal move generation */
void initialize_chess_engine_only_necessary() {
    initialize_oldstate_array();
    initialize_history_hash_array();
    initialize_attack_boards();
    initialize_helper_boards();
}
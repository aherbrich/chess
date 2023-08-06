#include <stdio.h>

#include "../include/move.h"
#include "../include/zobrist.h"
#include "../include/helpers.h"
#include "../include/pq.h"

const bitboard_t MASK_FILE[8] = {
	0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
	0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080
};

const bitboard_t CLEAR_FILE[8] = {
    18374403900871474942ULL, 18302063728033398269ULL, 18157383382357244923ULL, 17868022691004938231ULL,
    17289301308300324847ULL, 16131858542891098079ULL, 13816973012072644543ULL,  9187201950435737471ULL
};

const bitboard_t MASK_RANK[8] = {
	0xff, 0xff00, 0xff0000, 0xff000000,
	0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

const bitboard_t CLEAR_RANK[8] = {
    18446744073709551360ULL, 18446744073709486335ULL, 18446744073692839935ULL, 18446744069431361535ULL, 
    18446742978492891135ULL, 18446463698244468735ULL, 18374967954648334335ULL,    72057594037927935ULL
};

const bitboard_t MASK_DIAGONAL[15] = {
	0x80, 0x8040, 0x804020,
	0x80402010, 0x8040201008, 0x804020100804,
	0x80402010080402, 0x8040201008040201, 0x4020100804020100,
	0x2010080402010000, 0x1008040201000000, 0x804020100000000,
	0x402010000000000, 0x201000000000000, 0x100000000000000,
};

const bitboard_t MASK_ANTI_DIAGONAL[15] = {
	0x1, 0x102, 0x10204,
	0x1020408, 0x102040810, 0x10204081020,
	0x1020408102040, 0x102040810204080, 0x204081020408000,
	0x408102040800000, 0x810204080000000, 0x1020408000000000,
	0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

const bitboard_t SQUARE_BB[65] = {
	0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x100000000, 0x200000000, 0x400000000, 0x800000000,
	0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
	0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
	0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
	0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
	0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
	0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
	0x0
};

const bitboard_t UNIBOARD = 18446744073709551615ULL;

const int ROOK_BITS[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10,
                           10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10,
                           10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10,
                           11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
                           10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

const int BISHOP_BITS[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5,
                             5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
                             5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
                             5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

const uint64_t ROOK_MAGIC[64] = {
    0xa8002c000108020ULL,  0x4440200140003000ULL, 0x8080200010011880ULL,
    0x380180080141000ULL,  0x1a00060008211044ULL, 0x410001000a0c0008ULL,
    0x9500060004008100ULL, 0x100024284a20700ULL,  0x802140008000ULL,
    0x80c01002a00840ULL,   0x402004282011020ULL,  0x9862000820420050ULL,
    0x1001448011100ULL,    0x6432800200800400ULL, 0x40100010002000cULL,
    0x2800d0010c080ULL,    0x90c0008000803042ULL, 0x4010004000200041ULL,
    0x3010010200040ULL,    0xa40828028001000ULL,  0x123010008000430ULL,
    0x24008004020080ULL,   0x60040001104802ULL,   0x582200028400d1ULL,
    0x4000802080044000ULL, 0x408208200420308ULL,  0x610038080102000ULL,
    0x3601000900100020ULL, 0x80080040180ULL,      0xc2020080040080ULL,
    0x80084400100102ULL,   0x4022408200014401ULL, 0x40052040800082ULL,
    0xb08200280804000ULL,  0x8a80a008801000ULL,   0x4000480080801000ULL,
    0x911808800801401ULL,  0x822a003002001894ULL, 0x401068091400108aULL,
    0x4a10a00004cULL,      0x2000800640008024ULL, 0x1486408102020020ULL,
    0x100a000d50041ULL,    0x810050020b0020ULL,   0x204000800808004ULL,
    0x20048100a000cULL,    0x112000831020004ULL,  0x9000040810002ULL,
    0x440490200208200ULL,  0x8910401000200040ULL, 0x6404200050008480ULL,
    0x4b824a2010010100ULL, 0x4080801810c0080ULL,  0x400802a0080ULL,
    0x8224080110026400ULL, 0x40002c4104088200ULL, 0x1002100104a0282ULL,
    0x1208400811048021ULL, 0x3201014a40d02001ULL, 0x5100019200501ULL,
    0x101000208001005ULL,  0x2008450080702ULL,    0x1002080301d00cULL,
    0x410201ce5c030092ULL,
};

const uint64_t BISHOP_MAGIC[64] = {
    0x40210414004040ULL,   0x2290100115012200ULL, 0xa240400a6004201ULL,
    0x80a0420800480ULL,    0x4022021000000061ULL, 0x31012010200000ULL,
    0x4404421051080068ULL, 0x1040882015000ULL,    0x8048c01206021210ULL,
    0x222091024088820ULL,  0x4328110102020200ULL, 0x901cc41052000d0ULL,
    0xa828c20210000200ULL, 0x308419004a004e0ULL,  0x4000840404860881ULL,
    0x800008424020680ULL,  0x28100040100204a1ULL, 0x82001002080510ULL,
    0x9008103000204010ULL, 0x141820040c00b000ULL, 0x81010090402022ULL,
    0x14400480602000ULL,   0x8a008048443c00ULL,   0x280202060220ULL,
    0x3520100860841100ULL, 0x9810083c02080100ULL, 0x41003000620c0140ULL,
    0x6100400104010a0ULL,  0x20840000802008ULL,   0x40050a010900a080ULL,
    0x818404001041602ULL,  0x8040604006010400ULL, 0x1028044001041800ULL,
    0x80b00828108200ULL,   0xc000280c04080220ULL, 0x3010020080880081ULL,
    0x10004c0400004100ULL, 0x3010020200002080ULL, 0x202304019004020aULL,
    0x4208a0000e110ULL,    0x108018410006000ULL,  0x202210120440800ULL,
    0x100850c828001000ULL, 0x1401024204800800ULL, 0x41028800402ULL,
    0x20642300480600ULL,   0x20410200800202ULL,   0xca02480845000080ULL,
    0x140c404a0080410ULL,  0x2180a40108884441ULL, 0x4410420104980302ULL,
    0x1108040046080000ULL, 0x8141029012020008ULL, 0x894081818082800ULL,
    0x40020404628000ULL,   0x804100c010c2122ULL,  0x8168210510101200ULL,
    0x1088148121080ULL,    0x204010100c11010ULL,  0x1814102013841400ULL,
    0xc00010020602ULL,     0x1045220c040820ULL,   0x12400808070840ULL,
    0x2004012a040132ULL,
};

const bitboard_t PAWN_ATTACK[2][64] = {{
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
	0x2, 0x5, 0xa, 0x14,
	0x28, 0x50, 0xa0, 0x40,
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
}, {
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
	0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
	0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
}
};

bitboard_t ROOK_ATTACK[64][4096];
bitboard_t BISHOP_ATTACK[64][4096];
bitboard_t KNIGHT_ATTACK[64];
bitboard_t KING_ATTACK[64];
bitboard_t ROOK_ATTACK_MASK[64];
bitboard_t BISHOP_ATTACK_MASK[64];

bitboard_t SQUARES_BETWEEN_BB[64][64];
bitboard_t LINE[64][64];

/////////////////////////////////////////////////////////////////////////////
////	FUNCTIONS CONCERNING INITILIZATION OF PRECALCULATED BOARDS

/* Calculates key to access the correct attack map in hashtable */
int transform(bitboard_t mask, uint64_t magic, int bits) {
    /* depending on the blockers mask, we need a different key to access the
     * correct attack map */
    return (int)((mask * magic) >> (64 - bits));
}

/* 1 to 1 mapping between integer of bit length n and blocking mask with n bits
 * set */
bitboard_t index_to_bitboard(int index, int n, bitboard_t mask) {
    int j;
    bitboard_t blocking_mask = 0ULL;
    for (int i = 0; i < n; i++) {
        j = pop_1st_bit(&mask);
        /* if i'th bit in number (index) is set */
        if (index & (1 << i)) {
            /* then carry over i'th 1-bit (namely the j'th bit in mask) from
             * mask to blocking mask */
            blocking_mask |= (1ULL << j);
        }
    }
    return blocking_mask;
}

/* calculates rook attack lines excluding edge/corner squares */
bitboard_t rook_mask(int sq) {
    bitboard_t result = 0ULL;
    int row = sq / 8;
    int col = sq % 8;
    /* north */
    for (int r = row + 1; r <= 6; r++) {
        result |= (1ULL << (col + r * 8));
    }
    /* south */
    for (int r = row - 1; r >= 1; r--) {
        result |= (1ULL << (col + r * 8));
    }
    /* east */
    for (int f = col + 1; f <= 6; f++) {
        result |= (1ULL << (f + row * 8));
    }
    /* west */
    for (int f = col - 1; f >= 1; f--) {
        result |= (1ULL << (f + row * 8));
    }

    return result;
}

/* calculates bishop attack lines excluding edge/corner squares */
bitboard_t bishop_mask(int sq) {
    bitboard_t result = 0ULL;
    int row = sq / 8;
    int col = sq % 8;
    /* north east */
    for (int r = row + 1, f = col + 1; r <= 6 && f <= 6; r++, f++) {
        result |= (1ULL << (f + r * 8));
    }
    /* north west */
    for (int r = row + 1, f = col - 1; r <= 6 && f >= 1; r++, f--) {
        result |= (1ULL << (f + r * 8));
    }
    /* south east */
    for (int r = row - 1, f = col + 1; r >= 1 && f <= 6; r--, f++) {
        result |= (1ULL << (f + r * 8));
    }
    /* south west */
    for (int r = row - 1, f = col - 1; r >= 1 && f >= 1; r--, f--) {
        result |= (1ULL << (f + r * 8));
    }

    return result;
}

/* calculates attack map of a rook at a given square and blocking mask */
bitboard_t rook_attacks(int sq, bitboard_t block) {
    bitboard_t attacks = 0ULL;
    int row = sq / 8;
    int col = sq % 8;

    /* north */
    for (int r = row + 1; r <= 7; r++) {
        attacks |= (1ULL << (col + r * 8));
        if (block & (1ULL << (col + r * 8))) {
            break;
        }
    }
    /* south */
    for (int r = row - 1; r >= 0; r--) {
        attacks |= (1ULL << (col + r * 8));
        if (block & (1ULL << (col + r * 8))) {
            break;
        }
    }
    /* east */
    for (int f = col + 1; f <= 7; f++) {
        attacks |= (1ULL << (f + row * 8));
        if (block & (1ULL << (f + row * 8))) {
            break;
        }
    }
    /* west */
    for (int f = col - 1; f >= 0; f--) {
        attacks |= (1ULL << (f + row * 8));
        if (block & (1ULL << (f + row * 8))) {
            break;
        }
    }
    return attacks;
}

/* calculates attack map of a bishop at a given square and blocking mask */
bitboard_t bishop_attacks(int sq, bitboard_t block) {
    bitboard_t attacks = 0ULL;
    int row = sq / 8;
    int col = sq % 8;

    /* north east */
    for (int r = row + 1, f = col + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    /* north west */
    for (int r = row + 1, f = col - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    /* south east */
    for (int r = row - 1, f = col + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    /* south west */
    for (int r = row - 1, f = col - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    return attacks;
}

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

/* Initializes hashtables with line masks and sqaures between masks */
void initialize_helper_boards() {
    initialize_squares_between();
    initialize_line();
}

/* Initializes old state array */
void initialize_history_hash_array() {
    for (int i = 0; i < MAXPLIES; i++) {
        HISTORY_HASHES[i] = 0;
    }
}

/* Initializes all structures necessary for legal move generation */
void initialize_chess_engine_necessary() {
    initialize_history_hash_array();
    initialize_attack_boards();
    initialize_helper_boards();
    initialize_zobrist_table();
}

//////////////////////////////////////////////////////
///			BASIC MOVE STRUCT FUNCTIONS 

/* Determines if two moves are the same */
int is_same_move(move_t *move1, move_t *move2) {
    /* moves are considered same if their FROM & TO squares and FLAGS are the
    same we do not consider the value of a move */
    if (move1->from != move2->from) {
        return 0;
    }
    if (move1->to != move2->to) {
        return 0;
    }
    if (move1->flags != move2->flags) {
        return 0;
    }

    return 1;
}

/* Allocates memory for a move and sets fields accordingly  */
move_t *generate_move(idx_t from, idx_t to, flag_t flags, uint16_t value) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->from = from;
    move->to = to;
    move->flags = flags;
    move->value = value;

    return move;
}

/* Copies a move */
move_t *copy_move(move_t *move) {
    if (move == NULL) {
        /* this should not happen but lets just handle it */
        return NULL;
    }

    move_t *copy = (move_t *)malloc(sizeof(move_t));

    copy->from = move->from;
    copy->to = move->to;
    copy->flags = move->flags;
    copy->value = move->value;

    return copy;
}

/* Frees memory of move */
void free_move(move_t *move) {
    if (move) {
        free(move);
    }
}


///////////////////////////////////////////////////////////////
////		FUNCTIONS CONCERNING MOVE GENERATION

/* Shifts a bitboard in given direction */
bitboard_t shift(bitboard_t bb, int direction){
    switch(direction){
        case NORTH: return bb << 8;
        case SOUTH: return bb >> 8;
        case NORTH_NORTH: return bb << 16;
        case SOUTH_SOUTH: return bb >> 16;
        case EAST: return ((bb & CLEAR_FILE[H]) << 1);
        case WEST: return ((bb & CLEAR_FILE[A]) >> 1);
        case NORTH_EAST: return ((bb & CLEAR_FILE[H]) << 9);
        case NORTH_WEST: return ((bb & CLEAR_FILE[A]) << 7);
        case SOUTH_EAST: return ((bb & CLEAR_FILE[H]) >> 7);
        case SOUTH_WEST: return ((bb & CLEAR_FILE[A]) >> 9);
        default: 
            fprintf(stderr, "This should not happen!"); 
            exit(1);
    }
}

/* Returns combined bitboard of bishops and queens */
bitboard_t diagonal_sliders(board_t* board, player_t player) {
	return (player == WHITE) ? board->piece_bb[W_BISHOP] | board->piece_bb[W_QUEEN] :
		board->piece_bb[B_BISHOP] | board->piece_bb[B_QUEEN];
}

/* Returns combined bitboard of rooks and queens */
bitboard_t orthogonal_sliders(board_t* board, player_t player) {
	return (player == WHITE) ? board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] :
		board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN];
}

/* Returns bitboard of squares that given pawns (plurals!) can attack on */
bitboard_t attack_pawn_multiple(bitboard_t p, player_t player) {
	return player == WHITE ? shift(p, NORTH_WEST) | shift(p, NORTH_EAST) :
		shift(p, SOUTH_WEST) | shift(p, SOUTH_EAST);
}

/* Returns bitboard of squares that a given pawn can attack on */
bitboard_t attack_pawn_single(square_t sq, player_t player){
    return PAWN_ATTACK[player][sq];
}

/* Returns bitboard of squares that a given bishop can attack on, given bitboard of (potentially) blocking pieces */
bitboard_t attack_bishop(square_t sq, bitboard_t occ){
    int j = transform(occ & BISHOP_ATTACK_MASK[sq], BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
    return BISHOP_ATTACK[sq][j];
}

/* Returns bitboard of squares that a given rook can attack on, given bitboard of (potentially) blocking pieces */
bitboard_t attack_rook(square_t sq, bitboard_t occ){
    int j = transform(occ & ROOK_ATTACK_MASK[sq], ROOK_MAGIC[sq], ROOK_BITS[sq]);
    return ROOK_ATTACK[sq][j];
}

/* Returns bitboard of squares of pieces which can capture on given square as specified player */
bitboard_t attackers_from(board_t* board, square_t sq, bitboard_t occ, player_t player) {
	return (player == WHITE) ? 
        ((attack_pawn_single(sq, BLACK) & board->piece_bb[W_PAWN]) | 
        (KNIGHT_ATTACK[sq] & board->piece_bb[W_KNIGHT]) |
        (attack_bishop(sq, occ) & (board->piece_bb[W_BISHOP] | board->piece_bb[W_QUEEN])) |
        (attack_rook(sq, occ) & (board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN]))) :
        ((attack_pawn_single(sq, WHITE) & board->piece_bb[B_PAWN]) | 
        (KNIGHT_ATTACK[sq] & board->piece_bb[B_KNIGHT]) |
        (attack_bishop(sq, occ) & (board->piece_bb[B_BISHOP] | board->piece_bb[B_QUEEN])) |
        (attack_rook(sq, occ) & (board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN])));
}

/* Returns direction as in the view of white (i.e NORTH stays NORTH as white,
but NORTH as black will be returned as SOUTH) */
dir_t relative_dir(player_t player, dir_t d) {
	return (player == WHITE) ? (dir_t) d : (dir_t) -d;
}

/* Returns rank as in the view of white (i.e RANK1 stays RANK1 as white,
but RANK1 as black will be returned as RANK8)*/
rank_t relative_rank(player_t player, rank_t r) {
	return (player == WHITE) ? (rank_t) r : (rank_t) (RANK8 - r);
}

/* Returns true if player is allowed to castle shortside */
int oo_allowed(player_t player, flag_t cr){
    return (player == WHITE) ? (cr & SHORTSIDEW) : (cr & SHORTSIDEB);
}

/* Returns true if player is allowed to castle longside */
int ooo_allowed(player_t player, flag_t cr){
    return (player == WHITE) ? (cr & LONGSIDEW) : (cr & LONGSIDEB);
}

/* Returns bitboard mask of squares involved in shortside casteling */
/* i.e. the squares between rook and king */
bitboard_t oo_blockers_mask(player_t player) { 
	return (player == WHITE) ? 96ULL : 6917529027641081856ULL; 
}

/* Returns bitboard mask of squares involved in longside casteling */
/* i.e. the squares between rook and king */
bitboard_t ooo_blockers_mask(player_t player) { 
	return (player == WHITE) ? 14ULL : 1008806316530991104ULL; 
}

/* Returns bitboard to mask out the b1/b8 square */
/* i.e. since attacks on the b square are not relevant for casteling
we have to mask it out when calculating castleing moves */
bitboard_t ignore_ooo_danger_bfile(player_t player) { return player == WHITE ? 0x2 : 0x200000000000000; }

/* Return piece, given piece type and color */
square_t make_piece(player_t player, int pc){
    return (player == WHITE) ? (square_t) (0b1000 & pc) : (square_t) (pc);
}

/* Generates and adds moves to movelist given a from square
and a bitboard of target squares */
void make_moves_quiet(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), QUIET, 0));
}

/* Generates and adds moves to movelist given a from square
and a bitboard of target squares */
void make_moves_doubleep(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), DOUBLEP, 0));
}

/* Generates and adds moves to movelist given a from square
and a bitboard of target squares */
void make_moves_capture(maxpq_t *movelst, board_t* board, square_t from, bitboard_t targets){
    while (targets) {
        int p = pop_1st_bit(&targets);
        insert(movelst, generate_move(from, p, CAPTURE, board->piece_bb[p] * 100 + (KING_ID - (board->piece_bb[from] & 0b111))));
    }
}

/* Generates and adds moves to movelist given a from square
and a bitboard of target squares */
void make_moves_epcapture(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), EPCAPTURE, 0));
}

/* Generates and adds moves to movelist given a from square
and a bitboard of target squares */
void make_moves_promcaptures(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets){
        int idx = pop_1st_bit(&targets);
        insert(movelst, generate_move(from, idx, KCPROM, 2500));
        insert(movelst, generate_move(from, idx, BCPROM, 2600));
        insert(movelst, generate_move(from, idx, RCPROM, 2700));
        insert(movelst, generate_move(from, idx, QCPROM, 2800));
    }
}

void generate_legals(board_t* board, maxpq_t *movelst){
    player_t us = board->player;
    player_t them = SWITCHSIDES(us);

	bitboard_t us_bb = (us == WHITE) ? 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]) : 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]);
	bitboard_t them_bb = (us == WHITE) ? 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]) : 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]);
	bitboard_t all = us_bb | them_bb;

	square_t our_king_sq = (us == WHITE) ? find_1st_bit(board->piece_bb[W_KING]) : find_1st_bit(board->piece_bb[B_KING]);
	square_t their_king_sq = (us == WHITE) ? find_1st_bit(board->piece_bb[B_KING]) : find_1st_bit(board->piece_bb[W_KING]);

	bitboard_t our_diag_sliders_bb = diagonal_sliders(board, us);
	bitboard_t their_diag_sliders_bb = diagonal_sliders(board, them);
	bitboard_t our_orth_sliders_bb = orthogonal_sliders(board, us);
	bitboard_t their_orth_sliders_bb = orthogonal_sliders(board, them);

    bitboard_t our_pawns_bb = (us == WHITE) ? board->piece_bb[W_PAWN] : board->piece_bb[B_PAWN];
    bitboard_t our_knights_bb = (us == WHITE) ? board->piece_bb[W_KNIGHT] : board->piece_bb[B_KNIGHT];

    bitboard_t their_pawns_bb = (them == WHITE) ? board->piece_bb[W_PAWN] : board->piece_bb[B_PAWN];
    bitboard_t their_knights_bb = (them == WHITE) ? board->piece_bb[W_KNIGHT] : board->piece_bb[B_KNIGHT];

    /* General purpose bitboards for attacks, masks, etc. */
	bitboard_t b1, b2, b3;
	
	/* Squares the king can not move to */
	bitboard_t danger = 0ULL;

	/* For each enemy piece add it's attacks to the danger bitboards */
    /* Attacks by PAWNS and KINGS */
	danger |= attack_pawn_multiple(their_pawns_bb, them) | KING_ATTACK[their_king_sq];

    /* Attacks by KNIGHTS */
    b1 = their_knights_bb; 
	while (b1) danger |=  KNIGHT_ATTACK[pop_1st_bit(&b1)];

    /* Diagonal attacks by BISHOPS and QUEENS */
	/* NOTICE: we xor out the king square to secure that squares x-rayed are also included in attack mask */
    b1 = their_diag_sliders_bb;
	while (b1) danger |= attack_bishop(pop_1st_bit(&b1), all ^ SQUARE_BB[our_king_sq]);

    /* Orthogonal attacks by BISHOPS and QUEENS */
	/* NOTICE: we xor out the king square to secure that squares x-rayed are also included in attack mask */
    b1 = their_orth_sliders_bb;
	while (b1) danger |= attack_rook(pop_1st_bit(&b1), all ^ SQUARE_BB[our_king_sq]);

	/* King can move to surrounding squares, except attacked sqaures and squares which are blocked by own pieces */
	b1 = KING_ATTACK[our_king_sq] & ~(us_bb | danger);
    make_moves_quiet(movelst, our_king_sq, b1 & ~them_bb);
    make_moves_capture(movelst, board, our_king_sq, b1 & them_bb);

    /* The capture mask filters destination squares to those that contain an enemy piece that is checking the  
	king and must be captured */
	bitboard_t capture_mask;
	
	/* The quiet mask filter destination squares to those where pieces must be moved to block an incoming attack 
	to the king */
    bitboard_t quiet_mask;
	
	/* A general purpose square for storing destinations, etc. */
	square_t s;

    /* Checking pieces are identified by:
		1. Computing attacks from the king square
		2. Intersecting this attack bitboard with the enemy bitboard of that piece type 
	*/
	board->checkers = (KNIGHT_ATTACK[our_king_sq] & their_knights_bb) | (attack_pawn_single(our_king_sq, us) & their_pawns_bb);

	/* 	1. Compute sliding attacks from king square (excluding our pieces from the blockers mask,
		to find potential pins in the next step).
		2. Find bitboard of checking/pinning sliding pieces by intersecting with board in 1.
	*/
	bitboard_t candidates = (attack_rook(our_king_sq, them_bb) & their_orth_sliders_bb) | (attack_bishop(our_king_sq, them_bb) & their_diag_sliders_bb);

	/* The next bit of code determines checking and pinned pieces */
    board->pinned = 0;
	while (candidates) {
		/* Intersect the line between (potentially) checking piece and
		 our king with the bitboard of our pieces */
		s = pop_1st_bit(&candidates);
		b1 = SQUARES_BETWEEN_BB[our_king_sq][s] & us_bb;
		
		/* If we find no pieces on this line, the attacking piece is a checking piece*/
		if (b1 == 0) board->checkers ^= SQUARE_BB[s];

		/* Else if we find only one of our pieces on this line, it is a pinned piece */
		else if ((b1 & b1 - 1) == 0) board->pinned ^= b1;
	}

	bitboard_t not_pinned = ~board->pinned;

    switch (sparse_pop_count(board->checkers)) {
	case 2:
		/* DOUBLE CHECK */
		/* If there is a double check the only legal moves are king moves */
		return;
	case 1: {
		/* SINGLE CHECK */
		square_t checker_square = find_1st_bit(board->checkers);

		switch (board->playingfield[checker_square] &0b111) {
		case PAWN:
			/* If the checker is a pawn, we must check for ep moves that can capture it */
			/* This evaluates to true if the checking piece is the one which just double pushed */
			if (board->checkers == shift(SQUARE_BB[board->history[board->ply_no].epsq], relative_dir(us, SOUTH))) {
				/* We compute the bitboard of pawns which can ep capture the checking pawn */
				b1 = attack_pawn_single(board->history[board->ply_no].epsq, them) & our_pawns_bb & not_pinned;
                
                while (b1) insert(movelst, generate_move(pop_1st_bit(&b1), board->history[board->ply_no].epsq, EPCAPTURE, 0));
			}
			/* INTENTIONAL FALL THROUGH */
		case KNIGHT:
			/* If the checker is either a pawn or a knight the only legal moves are to capture
			the checker. Only non-pinned pieces can capture it */
			b1 = attackers_from(board, checker_square, all, us) & not_pinned;
			while (b1) {
                int p = pop_1st_bit(&b1);
                insert(movelst, generate_move(p , checker_square, CAPTURE, board->piece_bb[checker_square]*100 + (KING_ID - (board->piece_bb[p] & 0b111))));
            }
			return;
		default:
			/* Else, we can either...*/
			/* (1) CAPTURE the checking piece */
			capture_mask = board->checkers;
			
			/* (2) BLOCK the attack since it is guaranteed to be from a slider */
			quiet_mask = SQUARES_BETWEEN_BB[our_king_sq][checker_square];
			break;
		}

		break;
	}

    default: {
		/* NOT IN CHECK */
        
		/* We can capture any enemy piece */
		capture_mask = them_bb;
		
		/* We can play a quiet move to any square which is not occupied */
		quiet_mask = ~all;

		/* Special handling of possible ep captures */
		if (board->history[board->ply_no].epsq != NO_SQUARE) {
			/* Compute bitboard of pawns which could capture on ep square */
			b2 = attack_pawn_single(board->history[board->ply_no].epsq, them) & our_pawns_bb;
			b1 = b2 & not_pinned;
			while (b1) {
				s = pop_1st_bit(&b1);
				
				/* This piece of evil bit-fiddling magic prevents the infamous 'pseudo-pinned' e.p. case,
				where the pawn is not directly pinned, but on moving the pawn and capturing the enemy pawn
				e.p., a rook or queen attack to the king is revealed */
				
				/*
				.nbqkbnr
				ppp.pppp
				........
				r..pP..K
				........
				........
				PPPP.PPP
				RNBQ.BNR
				
				Here, if white plays exd5 e.p., the black rook on a5 attacks the white king on h5 
				*/

				/* We xor out (1) us and (2) the 'ep-pawn', then compute sliding attacks from the king square,
				mask this with the rank the king is standing on, and intersect this with orthogonal attackers of the enemy */
                if (((sliding_attacks(our_king_sq, 
                                        all ^ SQUARE_BB[s] ^ shift(SQUARE_BB[board->history[board->ply_no].epsq], relative_dir(us, SOUTH)), 
                                        MASK_RANK[rank_of(our_king_sq)]) 
                    & their_orth_sliders_bb) == 0)){
                        insert(movelst, generate_move(s, board->history[board->ply_no].epsq, EPCAPTURE, 0));
                }

				/* WARNING: The same situation for diagonal attacks (see "8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1") is not handled.
				In a normal game of chess, positions like the one above can not occur legally. 
				!!! Be warned that move generation on custom made positions might be wrong !!!
				Stockfish, for example, still handles positions like this though.
				*/
			}
			
			/* Pinned pawns can only ep capture if they are pinned diagonally
			i.e. and the e.p. square is in line with the king */
			b1 = b2 & board->pinned & LINE[board->history[board->ply_no].epsq][our_king_sq];
			if (b1) {
                insert(movelst, generate_move(find_1st_bit(b1), board->history[board->ply_no].epsq, EPCAPTURE, 0));
			}
		}

		/* Only add castleing if:
			1. We have the corresponding castle rights (the rook and king have no moved before)
			2. No piece is blocking in between the king and rook
			3. The king is not in check, moving through check or lands in check
		*/
        if(!((all | danger) & oo_blockers_mask(us)) && (oo_allowed(us, board->history[board->ply_no].castlerights))){
            if(us == WHITE){
                insert(movelst, generate_move(e1, g1, KCASTLE, 0));
            } else{
                insert(movelst, generate_move(e8, g8, KCASTLE, 0));
            }
        }
		/* NOTICE: since attacks on the b square are not relevant for casteling
			we have to mask it out when calculating castleing moves */
        if(!((all | (danger & ~ignore_ooo_danger_bfile(us))) & ooo_blockers_mask(us)) && (ooo_allowed(us, board->history[board->ply_no].castlerights))){
            if(us == WHITE){
                insert(movelst, generate_move(e1, c1, QCASTLE, 0));
            } else{
                insert(movelst, generate_move(e8, c8, QCASTLE, 0));
            }
        }

		/* Compute moves for PINNED pieces */
		/* Pinned BISHOPS, ROOK, QUEEN */
		b1 = ~(not_pinned | our_knights_bb | our_pawns_bb);
		while (b1) {
			s = pop_1st_bit(&b1);
			
			/* Since, the pieces are pinned we only generate moves for which the moving piece
			does not leave the line between king and checking piece */
            switch(board->playingfield[s] & 0b111){
                case BISHOP:
                    b2 = attack_bishop(s, all) & LINE[our_king_sq][s];
                    break;
                case ROOK:
                    b2 = attack_rook(s, all) & LINE[our_king_sq][s];
                    break;
                case QUEEN:
                    b2 = (attack_bishop(s, all) | attack_rook(s, all)) & LINE[our_king_sq][s];
            }
            make_moves_quiet(movelst, s, b2 & quiet_mask);
            make_moves_capture(movelst, board,  s, b2 & capture_mask);
		}

		/* Pinned PAWN */
		b1 = ~not_pinned & our_pawns_bb;
		while (b1) {
			s = pop_1st_bit(&b1);

			if (rank_of(s) == relative_rank(us, RANK7)) {
				/* Quiet promotions are impossible since the square in front of the pawn will
				either be occupied by the king or the pinner, or doing so would leave our king
				in check */
				b2 = attack_pawn_single(s, us) & capture_mask & LINE[our_king_sq][s];
                make_moves_promcaptures(movelst, s, b2);
			}
			else {
				/* Captures */
                b2 = attack_pawn_single(s, us) & capture_mask & LINE[our_king_sq][s];
                make_moves_capture(movelst, board, s, b2);
				
				/* Single pawn pushes */
				b2 = shift(SQUARE_BB[s], relative_dir(us, NORTH)) & ~all & LINE[our_king_sq][s];
				
				/* Double pawn pushes */ 
				/* only pawns on rank 3/6 are eligible */
				b3 = shift(b2 & MASK_RANK[relative_rank(us, RANK3)], 
                                relative_dir(us, NORTH)) & ~all & LINE[our_king_sq][s];
                make_moves_quiet(movelst, s, b2);
                make_moves_doubleep(movelst, s, b3);
			}
		}
		
		/* Pinned KNIGHTS cannot move anywhere, so we're done with pinned pieces! */

		break;
    }
    
    }

    /* Non-pinned KNIGHT moves */
	b1 = our_knights_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = KNIGHT_ATTACK[s];
        make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}

    /* Non-pinned BISHOPS and QUEENS */
	b1 = our_diag_sliders_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = attack_bishop(s, all);
        make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}

    /* Non-pinned ROOKS and QUEENS */
	b1 = our_orth_sliders_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = attack_rook(s, all);
		make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}


    /* Determine pawns which are NOT about to promote */
	b1 = our_pawns_bb & not_pinned & ~MASK_RANK[relative_rank(us, RANK7)];

    /* Single pawn pushes */
	b2 = shift(b1, relative_dir(us, NORTH)) & ~all;
	
	/* Double pawn pushes */ 
	/* only pawns on rank 3/6 are eligible */
	b3 = shift(b2 & MASK_RANK[relative_rank(us, RANK3)], relative_dir(us, NORTH)) & quiet_mask;

    /* We &(and) b2 with the quiet mask only later, as a non-check-blocking single push does NOT mean that the 
	corresponding double push is not blocking check either. */
	b2 &= quiet_mask;

    while (b2) {
		s = pop_1st_bit(&b2);
        insert(movelst, generate_move(s-relative_dir(us, NORTH), s, QUIET, 0));
	}

	while (b3) {
		s = pop_1st_bit(&b3);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_NORTH), s, DOUBLEP, 0));
	}

    /* Pawn captures */
	b2 = shift(b1, relative_dir(us, NORTH_WEST)) & capture_mask;
	b3 = shift(b1, relative_dir(us, NORTH_EAST)) & capture_mask;

	while (b2) {
		s = pop_1st_bit(&b2);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, CAPTURE, board->piece_bb[s]*100 + (KING_ID - PAWN_ID)));
	}

	while (b3) {
		s = pop_1st_bit(&b3);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, CAPTURE, board->piece_bb[s]*100 + (KING_ID - PAWN_ID)));
	}

    /* Determine pawns which are about to promote */
	b1 = our_pawns_bb & not_pinned & MASK_RANK[relative_rank(us, RANK7)];
	if (b1) {
		/* Quiet promotions */
		b2 = shift(b1, relative_dir(us, NORTH)) & quiet_mask;
		while (b2) {
			s = pop_1st_bit(&b2);
			/* One move is added for each promotion piece */
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, KPROM, 2100));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, BPROM, 2200));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, RPROM, 2300));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, QPROM, 2400));
		}

		/* Capture promotions */
		b2 = shift(b1, relative_dir(us, NORTH_WEST)) & capture_mask;
		b3 = shift(b1, relative_dir(us, NORTH_EAST)) & capture_mask;

		while (b2) {
			s = pop_1st_bit(&b2);
			/* One move is added for each promotion piece */
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, KCPROM, 2500));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, BCPROM, 2600));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, RCPROM, 2700));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, QCPROM, 2800));
		}

		while (b3) {
			s = pop_1st_bit(&b3);
			/* One move is added for each promotion piece */
			insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, KCPROM, 2500));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, BCPROM, 2600));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, RCPROM, 2700));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, QCPROM, 2800));
		}
	}

}

/* Generates all legal moves for player at turn */
void generate_moves(board_t *board, maxpq_t *movelst) {
    generate_legals(board, movelst);
}


///////////////////////////////////////////////////////////////
////		FUNCTIONS CONCERNING MOVE EXECUTION

void remove_piece(board_t* board, square_t sq){
    board->hash ^= zobrist_table.piece_random64[board->playingfield[sq]][sq];
    board->piece_bb[board->playingfield[sq]] &= ~SQUARE_BB[sq];
	board->playingfield[sq] = NO_PIECE;
}

void put_piece(board_t* board, piece_t pc, square_t sq){
    board->piece_bb[pc] |= SQUARE_BB[sq];
    board->playingfield[sq] = pc;
    board->hash ^= zobrist_table.piece_random64[pc][sq];
}

void move_piece(board_t* board, square_t from, square_t to) {
    if(board->playingfield[to] == NO_PIECE) exit(3);
    board->hash ^= zobrist_table.piece_random64[board->playingfield[from]][from] ^ 
                    zobrist_table.piece_random64[board->playingfield[from]][to] ^
                    zobrist_table.piece_random64[board->playingfield[to]][to];
	bitboard_t mask = SQUARE_BB[from] | SQUARE_BB[to];
	board->piece_bb[board->playingfield[from]] ^= mask;
	board->piece_bb[board->playingfield[to]] &= ~mask;
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

void move_piece_quiet(board_t* board, square_t from, square_t to) {
    board->hash ^= zobrist_table.piece_random64[board->playingfield[from]][from] ^ 
                    zobrist_table.piece_random64[board->playingfield[from]][to];
	board->piece_bb[board->playingfield[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

/* Execute move */
void do_move(board_t *board, move_t *move) {
    /* save current board hash in array */
    HISTORY_HASHES[board->ply_no] = board->hash;
    
    /* increase board ply number */
    board->ply_no++;
    uint16_t ply = board->ply_no;

    /* reset history fields for board at new ply */
    board->history[ply].captured = NO_PIECE;
    board->history[ply].epsq = NO_SQUARE;
    board->history[ply].castlerights = board->history[ply-1].castlerights;
    board->history[ply].fifty_move_counter = board->history[ply-1].fifty_move_counter;
    board->history[ply].full_move_counter = board->history[ply-1].full_move_counter;

    /* adjustment of zobrist hash */
    /* xor out the (old) ep square if an ep sqaure was given i.e. epcapture was possible at ply-1*/
    if(board->history[ply-1].epsq != NO_SQUARE) board->hash ^= zobrist_table.flag_random64[board->history[ply-1].epsq % 8];
    /* xor out the (old) castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[ply-1].castlerights+8];

    /* reset fifty-counter if move is a capture or a pawn move*/
    if ((move->flags & 0b0100) || (SQUARE_BB[move->from] & board->piece_bb[W_PAWN]) ||
        (SQUARE_BB[move->from] & board->piece_bb[B_PAWN])) {
        board->history[ply].fifty_move_counter = 0;

    } else {
        /* else increase it */
        board->history[ply].fifty_move_counter++;
    }

    /* if black is making the move/ made his move, then increase the full move
     * counter */
    if (board->player == BLACK) {
        board->history[ply].full_move_counter++;
    }

    /* adjust castling rights if (potentially) king or rook moved from their start squares */
    if (move->from == a1) board->history[ply].castlerights &= ~(LONGSIDEW);
    else if (move->from == h1) board->history[ply].castlerights &= ~(SHORTSIDEW);
    else if (move->from == a8) board->history[ply].castlerights &= ~(LONGSIDEB);
    else if (move->from == h8) board->history[ply].castlerights &= ~(SHORTSIDEB);
    else if(move->from == e1) board->history[ply].castlerights &= ~(SHORTSIDEW | LONGSIDEW);
    else if(move->from == e8) board->history[ply].castlerights &= ~(SHORTSIDEB | LONGSIDEB);

    

    /* adjust castle rights if rooks were (potentially) captured on their start squares */
    if (move->to == h1) board->history[ply].castlerights &= ~(SHORTSIDEW);
    else if (move->to == a1) board->history[ply].castlerights &= ~(LONGSIDEW);
    else if (move->to == h8) board->history[ply].castlerights &= ~(SHORTSIDEB);
    else if (move->to == a8) board->history[ply].castlerights &= ~(LONGSIDEB);


    moveflags_t type = move->flags;
	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->from, move->to);
		break;
	case DOUBLEP:
		move_piece_quiet(board, move->from, move->to);
        if (board->player == WHITE) {
            board->history[ply].epsq = move->from + 8;
        } else {
            board->history[ply].epsq = move->from - 8;
        }
        /* xor in the new ep square */
        board->hash ^= zobrist_table.flag_random64[board->history[ply].epsq % 8];
		break;
	case KCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, g1);
			move_piece_quiet(board, h1, f1);
            board->history[ply].castlerights &= ~(SHORTSIDEW | LONGSIDEW);
		} else {
			move_piece_quiet(board, e8, g8);
			move_piece_quiet(board, h8, f8);
            board->history[ply].castlerights &= ~(SHORTSIDEB | LONGSIDEB);
		}			
		break;
	case QCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, c1); 
			move_piece_quiet(board, a1, d1);
            board->history[ply].castlerights &= ~(LONGSIDEW | SHORTSIDEW);
		} else {
			move_piece_quiet(board, e8, c8);
			move_piece_quiet(board, a8, d8);
            board->history[ply].castlerights &= ~(LONGSIDEB | SHORTSIDEB);
		}
		break;
	case EPCAPTURE:
		move_piece_quiet(board, move->from, move->to);

        if (board->player == WHITE) {
            board->history[ply-1].captured = B_PAWN;
            remove_piece(board, move->to - 8);
        } else {
            board->history[ply-1].captured = W_PAWN;
            remove_piece(board, move->to + 8);
        }
		break;
	case KPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_KNIGHT, move->to);
        } else {
            put_piece(board, B_KNIGHT, move->to);
        }
		break;
	case BPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_BISHOP, move->to);
        } else {
            put_piece(board, B_BISHOP, move->to);
        }
		break;
	case RPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_ROOK, move->to);
        } else {
            put_piece(board, B_ROOK, move->to);
        }
		break;
	case QPROM:
        remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_QUEEN, move->to);
        } else {
            put_piece(board, B_QUEEN, move->to);
        }
		break;
	case KCPROM:
        board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_KNIGHT, move->to);
        } else {
            put_piece(board, B_KNIGHT, move->to);
        }
		break;
	case BCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_BISHOP, move->to);
        } else {
            put_piece(board, B_BISHOP, move->to);
        }
		break;
	case RCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

       if (board->player == WHITE) {
            put_piece(board, W_ROOK, move->to);
        } else {
            put_piece(board, B_ROOK, move->to);
        }
		break;
	case QCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_QUEEN, move->to);
        } else {
            put_piece(board, B_QUEEN, move->to);
        }
		break;
	case CAPTURE:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];
		move_piece(board, move->from, move->to);
		break;
    default:
        fprintf(stderr,
            "This should not happen, since all types of moves should have been "
            "checked exhaustively\n");
        exit(1);
	}
    
    board->player = SWITCHSIDES(board->player);
    /* xor out old player and xor in the new player */
    board->hash ^= zobrist_table.flag_random64[24] ^ zobrist_table.flag_random64[25];
    /* xor in the new castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[ply].castlerights+8];
}

/* Undos a move */
void undo_move(board_t *board, move_t* move) {
    /* xor out old castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].castlerights+8];
    /* reduce ply number */
    board->ply_no--;
    /* xor in new castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].castlerights+8];
    /* xor out the (old) ep square if an ep sqaure was given i.e. epcapture was possible at ply+1*/
    if(board->history[board->ply_no].epsq != NO_SQUARE) board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].epsq % 8];

    moveflags_t type = move->flags;

	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->to, move->from);
		break;
	case DOUBLEP:
    /* xor in new ep square */
        board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no+1].epsq % 8];
		move_piece_quiet(board, move->to, move->from);
		break;
	case KCASTLE:
		if (board->player == BLACK) {
			move_piece_quiet(board, g1, e1);
			move_piece_quiet(board, f1, h1);
		} else {
			move_piece_quiet(board, g8, e8);
			move_piece_quiet(board, f8, h8);
		}
		break;
	case QCASTLE:
		if (board->player == BLACK) {
			move_piece_quiet(board, c1, e1);
			move_piece_quiet(board, d1, a1);
		} else {
			move_piece_quiet(board, c8, e8);
			move_piece_quiet(board, d8, a8);
		}
		break;
	case EPCAPTURE:
		move_piece_quiet(board, move->to, move->from);
        if(board->player == WHITE){
            put_piece(board, W_PAWN, move->to + 8);
        } else{
            put_piece(board, B_PAWN, move->to - 8);
        }
		break;
	case KPROM:
	case BPROM:
	case RPROM:
	case QPROM:
        remove_piece(board, move->to);
        if(board->player == WHITE){
            put_piece(board, B_PAWN, move->from);
        } else{
            put_piece(board, W_PAWN, move->from);
        }
		break;
	case KCPROM:
	case BCPROM:
	case RCPROM:
	case QCPROM:
        remove_piece(board, move->to);
        if(board->player == WHITE){
            put_piece(board, B_PAWN, move->from);
        } else{
            put_piece(board, W_PAWN, move->from);
        }
        put_piece(board, board->history[board->ply_no].captured, move->to);
		break;
	case CAPTURE:
		move_piece_quiet(board, move->to, move->from);
        put_piece(board, board->history[board->ply_no].captured, move->to);
		break;
	}

	board->player = SWITCHSIDES(board->player);
    /* xor in the new castle rights */
    board->hash ^= zobrist_table.flag_random64[24] ^ zobrist_table.flag_random64[25];
}


///////////////////////////////////////////////////////////////
////		FUNCTIONS CONCERNING CHECK VALIDITY

/* Checks if a player is in check WHEN AT TURN */
/* WARNING: Only call AFTER move generation (and BEFORE making move)
(since we determine if we are in check in move generation anyway,
but would switch players if we made move early) */
int is_in_check(board_t *board) {
	return(board->checkers);
}

/* Checks if the opponent player is in check after move */
/* WARNING: Call only AFTER making a move as player A
to check if player B is now in check */
int is_in_check_opponent(board_t* board){
    player_t us = board->player;
    player_t them = SWITCHSIDES(us);

	bitboard_t us_bb = (us == WHITE) ? 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]) : 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]);
	bitboard_t them_bb = (us == WHITE) ? 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]) : 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]);
	bitboard_t all_bb = us_bb | them_bb;

	square_t our_king_sq = (us == WHITE) ? find_1st_bit(board->piece_bb[W_KING]) : find_1st_bit(board->piece_bb[B_KING]);

	bitboard_t their_diag_sliders_bb = diagonal_sliders(board, them);
	bitboard_t their_orth_sliders_bb = orthogonal_sliders(board, them);
    bitboard_t their_pawns_bb = (them == WHITE) ? board->piece_bb[W_PAWN] : board->piece_bb[B_PAWN];
    bitboard_t their_knights_bb = (them == WHITE) ? board->piece_bb[W_KNIGHT] : board->piece_bb[B_KNIGHT];

    /* General purpose bitboards for attacks, masks, etc. */
    bitboard_t checkers = 0ULL;
	
	checkers = 	(KNIGHT_ATTACK[our_king_sq] & their_knights_bb) | 
				(attack_pawn_single(our_king_sq, us) & their_pawns_bb) | 
				(attack_rook(our_king_sq, all_bb) & their_orth_sliders_bb) | 
				(attack_bishop(our_king_sq, all_bb) & their_diag_sliders_bb);

    if(checkers) return 1;

    return 0;
}
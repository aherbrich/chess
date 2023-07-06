
#include "../include/chess.h"

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

/* Calculates key to access the correct attack map in hashtable */
int transform(bitboard_t mask, uint64_t magic, int bits) {
    // depending on the blockers mask, we need a different key to acces the
    // corrent attack map
    return (int)((mask * magic) >> (64 - bits));
}

/* 1 to 1 mapping between integer of bit length n and blocking mask with n bits
 * set */
bitboard_t index_to_bitboard(int index, int n, bitboard_t mask) {
    int j;
    bitboard_t blocking_mask = 0ULL;
    for (int i = 0; i < n; i++) {
        j = pop_1st_bit(&mask);
        // if i'th bit in number (index) is set
        if (index & (1 << i)) {
            // then carry over i'th 1-bit (namely the j'th bit in mask) from
            // mask to blocking mask
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
    // north
    for (int r = row + 1; r <= 6; r++) {
        result |= (1ULL << (col + r * 8));
    }
    // south
    for (int r = row - 1; r >= 1; r--) {
        result |= (1ULL << (col + r * 8));
    }
    // east
    for (int f = col + 1; f <= 6; f++) {
        result |= (1ULL << (f + row * 8));
    }
    // west
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
    // north east
    for (int r = row + 1, f = col + 1; r <= 6 && f <= 6; r++, f++) {
        result |= (1ULL << (f + r * 8));
    }
    // north west
    for (int r = row + 1, f = col - 1; r <= 6 && f >= 1; r++, f--) {
        result |= (1ULL << (f + r * 8));
    }
    // south east
    for (int r = row - 1, f = col + 1; r >= 1 && f <= 6; r--, f++) {
        result |= (1ULL << (f + r * 8));
    }
    // south west
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

    // north
    for (int r = row + 1; r <= 7; r++) {
        attacks |= (1ULL << (col + r * 8));
        if (block & (1ULL << (col + r * 8))) {
            break;
        }
    }
    // south
    for (int r = row - 1; r >= 0; r--) {
        attacks |= (1ULL << (col + r * 8));
        if (block & (1ULL << (col + r * 8))) {
            break;
        }
    }
    // east
    for (int f = col + 1; f <= 7; f++) {
        attacks |= (1ULL << (f + row * 8));
        if (block & (1ULL << (f + row * 8))) {
            break;
        }
    }
    // west
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

    // north east
    for (int r = row + 1, f = col + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    // north west
    for (int r = row + 1, f = col - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    // south east
    for (int r = row - 1, f = col + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    // south west
    for (int r = row - 1, f = col - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (f + r * 8));
        if (block & (1ULL << (f + r * 8))) {
            break;
        }
    }
    return attacks;
}

//
//  THE FOLLWOING FUNCTIONS ARE NOT USED AT RUNTIME
//  BUT ARE RATHER USED FOR THE PRECALCULATION OF
//  MAGIC NUMBERS AND BITBOARDS
//  (see ROOK_MAGIC and BISHOP_MAGIC above)
//

/* finds magic numbers by trail and error */
bitboard_t find_magic(int sq, int nr_bits, int for_bishop) {
    bitboard_t mask, magic;
    bitboard_t blockermap[4096], attackmap[4096], used[4096];

    mask = (for_bishop) ? bishop_mask(sq) : rook_mask(sq);

    // generate all possible blocker maps and the resulting attack maps for the
    // piece at sq
    for (int i = 0; i < (1 << nr_bits); i++) {
        blockermap[i] = index_to_bitboard(i, nr_bits, mask);
        attackmap[i] = (for_bishop) ? bishop_attacks(sq, blockermap[i])
                                    : rook_attacks(sq, blockermap[i]);
    }

    for (int k = 0; k < 100000000; k++) {
        magic = random_uint64_fewbits();

        for (int i = 0; i < 4096; i++) {
            used[i] = 0ULL;
        }

        int fail = 0;

        for (int i = 0; i < (1 << nr_bits); i++) {
            int j = transform(blockermap[i], magic, nr_bits);
            if (used[j] == 0ULL) {
                used[j] = attackmap[i];
            } else if (used[j] != attackmap[i]) {
                fail = 1;
                break;
            }
        }

        if (!fail) {
            return magic;
        }
    }
    fprintf(stderr, "*** Failed to find magic number ***\n");
    return 0ULL;
}

/* determines and prints all magic numbers (for all squares)*/
void print_magic_numbers() {
    int square;
    fprintf(stderr, "uint64_t ROOK_MAGIC[64] = {\n");
    for (square = 0; square < 64; square++)
        fprintf(stderr, "  0x%llxULL,\n",
                find_magic(square, ROOK_BITS[square], 0));
    fprintf(stderr, "};\n\n");

    fprintf(stderr, "uint64_t BISHOP_MAGIC[64] = {\n");
    for (square = 0; square < 64; square++)
        fprintf(stderr, "  0x%llxULL,\n",
                find_magic(square, BISHOP_BITS[square], 1));
    fprintf(stderr, "};\n\n");
}
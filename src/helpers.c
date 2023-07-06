#include "../include/chess.h"
#include "../include/magic.h"

/* Generates a pseudo-random 64bit unsigned integer */
uint64_t random_uint64() {
    uint64_t u1, u2, u3, u4;
    u1 = (uint64_t) (random()) & 0xFFFF;
    u2 = (uint64_t) (random()) & 0xFFFF;
    u3 = (uint64_t) (random()) & 0xFFFF;
    u4 = (uint64_t) (random()) & 0xFFFF;
    return (u1 | (u2 << 16) | (u3 << 32) | (u4 << 48));
}

/* Generates a pseudo-random 64bit unsigned integer with only few bits set */
uint64_t random_uint64_fewbits() {
    return random_uint64() & random_uint64() & random_uint64();
}

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

/* 
 * Initializes hashtables with attack board for each square
 * WARNING: initialize_helper_boards should always be called BEFORE 
 */
void initialize_attack_boards(){
    // precaution call (see WARNING above)
    initialize_helper_boards();

    // bishop attacks
    for(int sq = 0; sq < 64; sq++){
        bitboard_t mask = bishop_mask(sq);

        // pre-generate attack boards for each square and blocking mask with magic bitboards
        for(int i = 0; i < (1 << BISHOP_BITS[sq]); i++) {
            bitboard_t blockermap = index_to_bitboard(i, BISHOP_BITS[sq], mask);
            int j = transform(blockermap, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
            BISHOP_ATTACK[sq][j] = bishop_attacks(sq, blockermap);
        }
    }
    // rook attacks 
    for(int sq = 0; sq < 64; sq++){
        bitboard_t mask = rook_mask(sq);
        
        // pre-generate attack boards for each square and blocking mask with magic bitboards
        for(int i = 0; i < (1 << ROOK_BITS[sq]); i++) {
            bitboard_t blockermap = index_to_bitboard(i, ROOK_BITS[sq], mask);
            int j = transform(blockermap, ROOK_MAGIC[sq], ROOK_BITS[sq]);
            ROOK_ATTACK[sq][j] = rook_attacks(sq, blockermap);
        }
    }
    // knight attacks
    for(int sq = 0; sq < 64; sq++){
        // pre-generate attack boards for each square
        bitboard_t knight = (1ULL << sq);
        bitboard_t attacks = ((knight & CLEAR_FILE[A] & CLEAR_FILE [B]) << 6 | (knight & CLEAR_FILE[A] & CLEAR_FILE [B]) >> 10) |
                            ((knight & CLEAR_FILE[G] & CLEAR_FILE [H]) << 10 | (knight & CLEAR_FILE[G] & CLEAR_FILE [H]) >> 6) | 
                            ((knight & CLEAR_FILE[A]) << 15 | (knight & CLEAR_FILE[A]) >> 17) | 
                            ((knight & CLEAR_FILE[H]) << 17 | (knight & CLEAR_FILE[H]) >> 15);
        KNIGHT_ATTACK[sq] = attacks;
    }
    // king attacks
    for(int sq = 0; sq < 64; sq++){
        // pre-generate attack boards for each square
        bitboard_t king = (1ULL << sq);
        bitboard_t attacks = ((king & CLEAR_FILE[A]) >> 1 | (king & CLEAR_FILE[A]) >> 9 | (king & CLEAR_FILE[A]) << 7) |
                            ((king & CLEAR_FILE[H]) << 1 | (king & CLEAR_FILE[H]) << 9 | (king & CLEAR_FILE[H]) >> 7) |
                            ((king >> 8) | (king << 8));
        KING_ATTACK[sq] = attacks;
    }
    
}

/* 
 * Initializes hashtables with masks for ranks and files 
 * WARNING: Should always be called BEFORE initialize_attack_boards 
 */
void initialize_helper_boards(){
    // mask rank
    for(int i = 0; i < 8; i++){
        MASK_RANK[i] = 255ULL << (i*8);
    }

    // clear rank
    for(int i = 0; i < 8; i++){
        CLEAR_RANK[i] = ~MASK_RANK[i];
    }

    // mask file 
    for(int i = 0; i < 8; i++){
        bitboard_t mask = 0;
        for(int j = i; j < 64; j = j+8){
            mask |= (1ULL << j);
        }
        MASK_FILE[i] = mask;
    }

    // clear file
    for(int i = 0; i < 8; i++){
        CLEAR_FILE[i] = ~MASK_FILE[i];
    }

    // universal board
    UNIBOARD = 18446744073709551615ULL;
}

/* Initializes old state array */
void initialize_oldstate_array(){
    for(int i = 0; i < 512; i++){
        OLDSTATE[i] = 0;
    }
}

/* Initializes old state array */
void initialize_history_hash_array(){
    for(int i = 0; i < 512; i++){
        HISTORY_HASHES[i] = 0;
    }
}

/* Initializes all structures necessary for legal move generation */
void initialize_chess_engine_only_necessary(){
    initialize_oldstate_array();
    initialize_history_hash_array();
    initialize_helper_boards();
    initialize_attack_boards();
}
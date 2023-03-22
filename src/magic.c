
#include "../include/chess.h"

const int ROOK_BITS[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const int BISHOP_BITS[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

int transform(bitboard_t mask, uint64_t magic, int bits) {
    return (int)((mask * magic) >> (64 - bits));
}

uint64_t random_uint64() {
    uint64_t u1, u2, u3, u4;
    u1 = (uint64_t) (random()) & 0xFFFF;
    u2 = (uint64_t) (random()) & 0xFFFF;
    u3 = (uint64_t) (random()) & 0xFFFF;
    u4 = (uint64_t) (random()) & 0xFFFF;
    return (u1 | (u2 << 16) | (u3 << 32) | (u4 << 48));
}

uint64_t random_uint64_fewbits() {
    return random_uint64() & random_uint64() & random_uint64();
}

bitboard_t index_to_bitboard(int index, int bits, bitboard_t mask) {
  int j;
  bitboard_t blocking_mask = 0ULL;
  for(int i = 0; i < bits; i++) {
    j = pop_1st_bit(&mask);
    // if i'th bit in index is set
    if(index & (1 << i)){
        // then carry over i'th 1-bit (namely the j'th bit in mask) from mask to blocking mask
        blocking_mask |= (1ULL << j);
    }
  }
  return blocking_mask;
}

bitboard_t rook_mask(int sq) {
    bitboard_t result = 0ULL;
    int row = sq/8;
    int col = sq%8;
    // north
    for(int r = row+1; r <= 6; r++){
        result |= (1ULL << (col + r*8));
    }
    // south
    for(int r = row-1; r >= 1; r--){
        result |= (1ULL << (col + r*8));
    }
    // east 
    for(int f = col+1; f <= 6; f++){
        result |= (1ULL << (f + row*8));
    }
    // west
    for(int f = col-1; f >= 1; f--){
        result |= (1ULL << (f + row*8));
    }

    return result;
}

bitboard_t bishop_mask(int sq) {
    bitboard_t result = 0ULL;
    int row = sq/8;
    int col = sq%8;
    // north east
    for(int r=row+1, f=col+1; r<=6 && f<=6; r++, f++){
        result |= (1ULL << (f + r*8));
    }
    // north west
    for(int r=row+1, f=col-1; r<=6 && f>=1; r++, f--){
        result |= (1ULL << (f + r*8));
    }
    // south east
    for(int r=row-1, f=col+1; r>=1 && f<=6; r--, f++){
        result |= (1ULL << (f + r*8));
    }
    // south west
    for(int r=row-1, f=col-1; r>=1 && f>=1; r--, f--){
        result |= (1ULL << (f + r*8));
    }

    return result;
}

bitboard_t rook_attacks(int sq, bitboard_t block) {
    bitboard_t attacks = 0ULL;
    int row = sq/8;
    int col = sq%8;

    // north
    for(int r = row+1; r <= 7; r++) {
        attacks |= (1ULL << (col + r*8));
        if(block & (1ULL << (col + r*8))){
            break;
        }
    }
    // south 
    for(int r = row-1; r >= 0; r--) {
        attacks |= (1ULL << (col + r*8));
        if(block & (1ULL << (col + r*8))){
            break;
        }
    }
    // east
    for(int f = col+1; f <= 7; f++) {
        attacks |= (1ULL << (f + row*8));
        if(block & (1ULL << (f + row*8))){
            break;
        }
    }
    // west
    for(int f = col-1; f >= 0; f--) {
        attacks |= (1ULL << (f + row*8));
        if(block & (1ULL << (f + row*8))){
            break;
        }
    }
    return attacks;
}

bitboard_t bishop_attacks(int sq, bitboard_t block) {
    bitboard_t attacks = 0ULL;
    int row = sq/8;
    int col = sq%8;

    // north east
    for(int r = row+1, f = col+1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))){
            break;
        }
    }
    // north west
    for(int r = row+1, f = col-1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))){
            break;
        }
    }
    // south east
    for(int r = row-1, f = col+1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))){
            break;
        }
    }
    // south west
    for(int r = row-1, f = col-1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))){
            break;
        }
    }
    return attacks;
}

bitboard_t find_magic(int sq, int nr_bits, int for_bishop) {
    bitboard_t mask, magic;
    bitboard_t blockermap[4096], attackmap[4096], used[4096];

    mask = (for_bishop)? bishop_mask(sq) : rook_mask(sq);

    // generate all possible blocker maps and the resulting attack maps for the piece at sq
    for(int i = 0; i < (1 << nr_bits); i++) {
        blockermap[i] = index_to_bitboard(i, nr_bits, mask);
        attackmap[i] = (for_bishop) ? bishop_attacks(sq, blockermap[i]) : rook_attacks(sq, blockermap[i]);
    }

    for(int k = 0; k < 100000000; k++) {
        magic = random_uint64_fewbits();

        for(int i = 0; i < 4096; i++){
            used[i] = 0ULL;
        }

        int fail = 0;

        for(int i = 0; i < (1 << nr_bits); i++) {
            
            int j = transform(blockermap[i], magic, nr_bits);
            if(used[j] == 0ULL){
                used[j] = attackmap[i];
            }  
            else if(used[j] != attackmap[i]){
                fail = 1;
                break;
            }
        }

        if(!fail){
            return magic;
        }

    }
    fprintf(stderr, "*** Failed to find magic number ***\n");
    return 0ULL;
}

void print_magic_numbers(){
    int square;
    fprintf(stderr, "uint64_t ROOK_MAGIC[64] = {\n");
    for(square = 0; square < 64; square++)
        fprintf(stderr, "  0x%llxULL,\n", find_magic(square, ROOK_BITS[square], 0));
    fprintf(stderr, "};\n\n");

    fprintf(stderr, "uint64_t BISHOP_MAGIC[64] = {\n");
    for(square = 0; square < 64; square++)
        fprintf(stderr, "  0x%llxULL,\n", find_magic(square, BISHOP_BITS[square], 1));
    fprintf(stderr, "};\n\n");
}
    

#ifndef __MAGIC_H__
#define __MAGIC_H__

#include "../include/chess.h"

/////////////////////////////////////////////////////////////
//  GLOBALS
extern const int ROOK_BITS[64];
extern const int BISHOP_BITS[64];
extern const uint64_t ROOK_MAGIC[64];
extern const uint64_t BISHOP_MAGIC[64];

/////////////////////////////////////////////////////////////
//  MAGIC BITBOARDS
int transform(bitboard_t mask, uint64_t magic, int bits);
bitboard_t index_to_bitboard(int index, int bits, bitboard_t mask);
bitboard_t rook_mask(int sq);
bitboard_t bishop_mask(int sq);
bitboard_t rook_attacks(int sq, bitboard_t block);
bitboard_t bishop_attacks(int sq, bitboard_t block);

#endif
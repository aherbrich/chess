#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "../include/types.h"

//  HELPER FUNCTIONS
uint64_t random_uint64();
uint64_t random_uint64_fewbits();
idx_t pos_to_idx(int row, int col);
int find_1st_bit(bitboard_t bb);
int pop_1st_bit(bitboard_t* bitboard);
int sparse_pop_count(bitboard_t x);

rank_t rank_of(square_t s);
file_t file_of(square_t s);
int diagonal_of(square_t s);
int anti_diagonal_of(square_t s);

#endif
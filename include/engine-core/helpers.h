#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* helper functions for universal use                                                               */
/* ------------------------------------------------------------------------------------------------ */

/* generates pseudorandom 64-bit interger*/
uint64_t random_uint64(void);
/* generates pseudorandom 64-bit interger with few bits set to 1 */
uint64_t random_uint64_fewbits(void);
/* determines index given a row and column */
idx_t pos_to_idx(int row, int col);
/* determines index of first bit set to 1 */
int find_1st_bit(bitboard_t bb);
/* determines index of first bit set to 1 and sets it to 0 */
int pop_1st_bit(bitboard_t* bitboard);
/* determines number of bits set to 1 (efficiently if only few bits are set) */
int sparse_pop_count(bitboard_t x);

/* determines rank of a given square */
rank_t rank_of(square_t s);
/* determines file of a given square */
file_t file_of(square_t s);
/* determines diagonal of a given square */
int diagonal_of(square_t s);
/* determines anti-diagonal of a given square */
int anti_diagonal_of(square_t s);

#endif

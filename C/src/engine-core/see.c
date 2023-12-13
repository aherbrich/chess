#include <stdio.h>

#include "include/engine-core/see.h"

#include "include/engine-core/types.h"
#include "include/engine-core/move.h"
#include "include/engine-core/helpers.h"
#include "include/engine-core/prettyprint.h"

/* returns (single-bit)-bitboard of least valuable piece which is in a given bitboard */
bitboard_t get_least_valuable_piece(board_t* board, bitboard_t possible_defenders, int player, int *lv_piece) {
    /* iterate thorugh pieces from least to most valuable */
    for (int piece = PAWN + (player << 3); piece < KING + (player << 3); piece++){
        bitboard_t defenders_of_type = possible_defenders & board->piece_bb[piece];
        /* if there are any possible defenders of this piece type... */
        if (defenders_of_type) {
            /* remember which piece it was */
            *lv_piece = piece;
            /* return bitboard (NOT with all possible defenders of this type) 
            but only the one at the least significant bit */

            /* bit trick */
            /* example: bb = (1ULL << 2) | (1ULL << 1), then 
                        bb & -bb = (1ULL << 1)              */
            return defenders_of_type & -defenders_of_type; 
        }
    }
    /* if no possible defenders were found, return 0 (empty bitboard) */
    return 0ULL;
}

/* returns static exchange evaluation of a move */
int32_t see(board_t* board, move_t move) {
    /* for a more detailed explanation visit
       https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm */

    /* array with piece values */
    int value[14] = {100, 320, 330, 500, 900, 0, 0, 0, 100, 320, 330, 500, 900, 0};

    /* array in which free store exchange values */
    int32_t gain[32];
    /* depth of see-search */ 
    int d = 0;
    /* player starting exchange */
    int player = board->player;

    /* bitboard of pieces which can be x-rayed */
    bitboard_t may_block_ray = board->piece_bb[B_PAWN] | board->piece_bb[B_BISHOP] | board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[W_PAWN] | board->piece_bb[W_BISHOP] | board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN];
    /* occupied squares */
    bitboard_t occ = may_block_ray | board->piece_bb[B_KNIGHT] |  board->piece_bb[B_KING] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_KING];
    /* possible defenders/attackers of capture square (we ignore potential blocked xray attacks) */
    bitboard_t possible_defenders = attackers_from_both_sides(board, move.to, occ);

    /* bitboard with from square set */
    bitboard_t from_bb = 1ULL << move.from;
    /* piece standing on from square */
    int capturing_piece = board->playingfield[move.from];

    /* start exchange iteration */
    gain[d] = value[board->playingfield[move.to]];
    do {
        /* switch player */
        player = SWITCHSIDES(player);

        d++;
        gain[d] = value[capturing_piece] - gain[d-1];           
        if(gain[d] < 0 && -gain[d-1] < 0) break;            /* pruning, similar to beta cutoff */
        possible_defenders ^= from_bb;                      /* remove from square from possible defenders */
        occ ^= from_bb;                                     /* remove capturing piece from occupied mask */
        
        /* if capturing piece was being x-rayed, we have to check if now new attackers join the exchange */
        if (from_bb & may_block_ray) {
            /* add x-ray attackers to possible defenders */      
            possible_defenders |= consider_xray(board, move.to, find_1st_bit(from_bb), occ);
        }

        /* get least valuable piece which can defend the capture square */
        from_bb = get_least_valuable_piece(board, possible_defenders, player, &capturing_piece);
    } while (from_bb);

    while(--d){
        gain[d-1] = (gain[d] > -gain[d-1]) ? -gain[d] : gain[d-1];
    }

    return gain[0];
}

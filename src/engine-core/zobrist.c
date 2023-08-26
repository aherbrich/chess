#include "include/engine-core/zobrist.h"

#include "include/engine-core/helpers.h"
#include "include/engine-core/move.h"
#include "include/engine-core/types.h"

zobrist_t zobrist_table;

/* Initializes the global zobrist table */
void initialize_zobrist_table() {
    /* Initialize random 64 bit number for each piece on each square of the
     * board */
    for (int i = 0; i < 64; i++) {
        for (int piece = 0; piece < 14; piece++) {
            zobrist_table.piece_random64[piece][i] = random_uint64();
        }
    }
    /* Initialize random 64 bit number for every board flag */
    for (int i = 0; i < 26; i++) {
        zobrist_table.flag_random64[i] = random_uint64();
    }
}

/* Hashes a board using zobrist hashing */
uint64_t calculate_zobrist_hash(board_t *board) {
    uint64_t hash = 0ULL;

    bitboard_t pawns, knights, bishops, rooks, queens, king;

    /* hash the WHITE piece positions */

    pawns = board->piece_bb[B_PAWN];
    knights = board->piece_bb[B_KNIGHT];
    bishops = board->piece_bb[B_BISHOP];
    rooks = board->piece_bb[B_ROOK];
    queens = board->piece_bb[B_QUEEN];
    king = board->piece_bb[B_KING];

    while (pawns) {
        hash ^= zobrist_table.piece_random64[0][pop_1st_bit(&pawns)];
    }
    while (knights) {
        hash ^= zobrist_table.piece_random64[1][pop_1st_bit(&knights)];
    }
    while (bishops) {
        hash ^= zobrist_table.piece_random64[2][pop_1st_bit(&bishops)];
    }
    while (rooks) {
        hash ^= zobrist_table.piece_random64[3][pop_1st_bit(&rooks)];
    }
    while (queens) {
        hash ^= zobrist_table.piece_random64[4][pop_1st_bit(&queens)];
    }
    while (king) {
        hash ^= zobrist_table.piece_random64[5][pop_1st_bit(&king)];
    }

    /* hash the BLACK piece positions */

    pawns = board->piece_bb[W_PAWN];
    knights = board->piece_bb[W_KNIGHT];
    bishops = board->piece_bb[W_BISHOP];
    rooks = board->piece_bb[W_ROOK];
    queens = board->piece_bb[W_QUEEN];
    king = board->piece_bb[W_KING];

    while (pawns) {
        hash ^= zobrist_table.piece_random64[8][pop_1st_bit(&pawns)];
    }
    while (knights) {
        hash ^= zobrist_table.piece_random64[9][pop_1st_bit(&knights)];
    }
    while (bishops) {
        hash ^= zobrist_table.piece_random64[10][pop_1st_bit(&bishops)];
    }
    while (rooks) {
        hash ^= zobrist_table.piece_random64[11][pop_1st_bit(&rooks)];
    }
    while (queens) {
        hash ^= zobrist_table.piece_random64[12][pop_1st_bit(&queens)];
    }
    while (king) {
        hash ^= zobrist_table.piece_random64[13][pop_1st_bit(&king)];
    }

    /* hash the flags */
    if (board->history[board->ply_no].epsq != NO_SQUARE) {
        hash ^= zobrist_table.flag_random64[board->history[board->ply_no].epsq % 8];
    }

    hash ^= zobrist_table.flag_random64[8 + board->history[board->ply_no].castlerights];

    if (board->player == BLACK) {
        hash ^= zobrist_table.flag_random64[24];
    }

    if (board->player == WHITE) {
        hash ^= zobrist_table.flag_random64[25];
    }
    return hash;
}



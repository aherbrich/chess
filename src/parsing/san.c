#include <stdio.h>

#include "include/parse/parse.h"

#include "include/engine-core/move.h"
#include "include/engine-core/types.h"

#define AMBIG_BY_RANK -1
#define AMBIG_BY_FILE 1

/* returns true if c is a file (i.e a-h) */
int is_file(char c) { return (c >= 'a' && c <= 'h'); }

/* returns true if c is a chess piece (i.e B,K,N,Q,R) */
int is_piece(char c) { return (c == 'B' || c == 'K' || c == 'N' || c == 'Q' || c == 'R'); }

/* converts a string to an index */
idx_t str_to_idx(char file, char rank) {
    idx_t idx = 0;
    idx = idx + ((file) - 'a');
    idx = idx + (8 * (rank - '1'));
    return idx;
}

/* determines pawn move equal to the move described by flags */
move_t* find_pawn_move(board_t* board, char file1, char file2, char rank2,
                       int promotion, char promo_piece) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a pawn move */
        bitboard_t from_mask = 1ULL << move->from;
        if ((from_mask & board->piece_bb[W_PAWN]) ||
            (from_mask & board->piece_bb[B_PAWN])) {
            /* check if move is FROM the correct file and TO the correct square
             */
            if (move->to == to && (move->from % 8 + 'a') == file1) {
                /* if non-promotion */
                if (!promotion) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if promotion */
                else {
                    if (promo_piece == 'Q' &&
                        (move->flags == QPROM || move->flags == QCPROM)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                    if (promo_piece == 'R' &&
                        (move->flags == RPROM || move->flags == RCPROM)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                    if (promo_piece == 'B' &&
                        (move->flags == BPROM || move->flags == BCPROM)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                    if (promo_piece == 'N' &&
                        (move->flags == KPROM || move->flags == KCPROM)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* determines castle move equal to the move described by flags */
move_t* find_castle_move(board_t* board, int kingside) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];
        /* if kingside castle found */
        if (kingside && move->flags == KCASTLE) {
            move_t* copy = copy_move(move);
            free_pq(&move_lst);
            return copy;
        }
        /* if queen-side castle found */
        else if (!kingside && move->flags == QCASTLE) {
            move_t* copy = copy_move(move);
            free_pq(&move_lst);
            return copy;
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* Determines knight move equal to the move described by flags */
move_t* find_knight_move(board_t* board, char file1, char rank1, char file2,
                         char rank2, int single_ambiguous,
                         int double_ambiguous) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);
    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a knight move */
        bitboard_t from_mask = 1ULL << move->from;
        if (((from_mask & board->piece_bb[W_KNIGHT]) ||
             (from_mask & board->piece_bb[B_KNIGHT]))) {
            /* check if TO square is the correct TO square */
            if (move->to == to) {
                /* if move is unambiguous, we can instantly return */
                if (!single_ambiguous && !double_ambiguous) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if move is ambiguous by rank and file */
                else if (double_ambiguous) {
                    /* check if file AND rank of FROM square is correct */
                    if ((move->from % 8 + 'a') == file1 &&
                        (move->from / 8 + '1') == rank1) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                } else if (single_ambiguous) {
                    /* check if file OR rank of FROM square is correct */
                    if ((single_ambiguous == AMBIG_BY_FILE &&
                         (move->from % 8 + 'a') == file1) ||
                        (single_ambiguous == AMBIG_BY_RANK &&
                         (move->from / 8 + '1') == rank1)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* determines bishop move equal to the move described by flags */
move_t* find_bishop_move(board_t* board, char file1, char rank1, char file2,
                         char rank2, int single_ambiguous,
                         int double_ambiguous) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a bishop move */
        bitboard_t from_mask = 1ULL << move->from;
        if (((from_mask & board->piece_bb[W_BISHOP]) ||
             (from_mask & board->piece_bb[B_BISHOP]))) {
            /* check if TO square is the correct TO square */
            if (move->to == to) {
                /* if move is unambiguous, we can instantly return */
                if (!single_ambiguous && !double_ambiguous) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if move is ambiguous by rank and file */
                else if (double_ambiguous) {
                    /* check if file AND rank of FROM square is correct */
                    if ((move->from % 8 + 'a') == file1 &&
                        (move->from / 8 + '1') == rank1) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                } else if (single_ambiguous) {
                    /* check if file OR rank of FROM square is correct */
                    if ((single_ambiguous == AMBIG_BY_FILE &&
                         (move->from % 8 + 'a') == file1) ||
                        (single_ambiguous == AMBIG_BY_RANK &&
                         (move->from / 8 + '1') == rank1)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* determines rook move equal to the move described by flags */
move_t* find_rook_move(board_t* board, char file1, char rank1, char file2,
                       char rank2, int single_ambiguous, int double_ambiguous) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a rook move */
        bitboard_t from_mask = 1ULL << move->from;
        if (((from_mask & board->piece_bb[W_ROOK]) ||
             (from_mask & board->piece_bb[B_ROOK]))) {
            /* check if TO square is the correct TO square */
            if (move->to == to) {
                /* if move is unambiguous, we can instantly return */
                if (!single_ambiguous && !double_ambiguous) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if move is ambiguous by rank and file */
                else if (double_ambiguous) {
                    /* check if file AND rank of FROM square is correct */
                    if ((move->from % 8 + 'a') == file1 &&
                        (move->from / 8 + '1') == rank1) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                } else if (single_ambiguous) {
                    /* check if file OR rank of FROM square is correct */
                    if ((single_ambiguous == AMBIG_BY_FILE &&
                         (move->from % 8 + 'a') == file1) ||
                        (single_ambiguous == AMBIG_BY_RANK &&
                         (move->from / 8 + '1') == rank1)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* determines queen move equal to the move described by flags */
move_t* find_queen_move(board_t* board, char file1, char rank1, char file2,
                        char rank2, int single_ambiguous,
                        int double_ambiguous) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a queen move */
        bitboard_t from_mask = 1ULL << move->from;
        if (((from_mask & board->piece_bb[W_QUEEN]) ||
             (from_mask & board->piece_bb[B_QUEEN]))) {
            /* check if TO square is the correct TO square */
            if (move->to == to) {
                /* if move is unambiguous, we can instantly return */
                if (!single_ambiguous && !double_ambiguous) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if move is ambiguous by rank and file */
                else if (double_ambiguous) {
                    /* check if file AND rank of FROM square is correct */
                    if ((move->from % 8 + 'a') == file1 &&
                        (move->from / 8 + '1') == rank1) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                } else if (single_ambiguous) {
                    /* check if file OR rank of FROM square is correct */
                    if ((single_ambiguous == AMBIG_BY_FILE &&
                         (move->from % 8 + 'a') == file1) ||
                        (single_ambiguous == AMBIG_BY_RANK &&
                         (move->from / 8 + '1') == rank1)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* determines king move equal to the move described by flags */
move_t* find_king_move(board_t* board, char file1, char rank1, char file2,
                       char rank2, int single_ambiguous, int double_ambiguous) {
    /* generate all possible moves in the current position */
    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    /* determine index of to square */
    idx_t to = str_to_idx(file2, rank2);

    /* iterate through all moves */
    for (int i = 1; i < (&move_lst)->nr_elem + 1; i++) {
        move_t* move = (&move_lst)->array[i];

        /* check if move is a king move */
        bitboard_t from_mask = 1ULL << move->from;
        if (((from_mask & board->piece_bb[W_KING]) ||
             (from_mask & board->piece_bb[B_KING]))) {
            /* check if TO square is the correct TO square */
            if (move->to == to) {
                /* if move is unambiguous, we can instantly return */
                if (!single_ambiguous && !double_ambiguous) {
                    move_t* copy = copy_move(move);
                    free_pq(&move_lst);
                    return copy;
                }
                /* if move is ambiguous by rank and file */
                else if (double_ambiguous) {
                    /* check if file AND rank of FROM square is correct */
                    if ((move->from % 8 + 'a') == file1 &&
                        (move->from / 8 + '1') == rank1) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                } else if (single_ambiguous) {
                    /* check if file OR rank of FROM square is correct */
                    if ((single_ambiguous == AMBIG_BY_FILE &&
                         (move->from % 8 + 'a') == file1) ||
                        (single_ambiguous == AMBIG_BY_RANK &&
                         (move->from / 8 + '1') == rank1)) {
                        move_t* copy = copy_move(move);
                        free_pq(&move_lst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&move_lst);
    return NULL;
}

/* converts a string (short algebraic notation) to a move */
move_t* str_to_move(board_t* board, char* token) {
    move_t* move = NULL;
    int idx = 0;

    /* if pawn move */
    if (is_file(token[idx])) {
        char file1 = token[idx];
        char file2;
        char rank2;
        int promotion = 0;
        char promo_piece = '-';

        idx++;
        /* if capture */
        if (token[idx] == 'x') {
            idx++;
            file2 = token[idx];
            idx++;
            rank2 = token[idx];
        }
        /* if non-capture */
        else {
            file2 = file1;
            rank2 = token[idx];
        }
        idx++;
        /* if promotion */
        if (token[idx] == '=') {
            promotion = 1;
            idx++;
            promo_piece = token[idx];
        }

        /* find fitting move */
        move = find_pawn_move(board, file1, file2, rank2, promotion, promo_piece);
    }
    /* if piece move */
    else if (is_piece(token[idx])) {
        char file1;
        char file2;
        char rank1;
        char rank2;
        char piece = token[idx];
        int single_ambiguous = 0;  /* -1 if by rank, 1 if by file */
        int double_ambiguous = 0;

        idx++;
        if (is_file(token[idx])) {
            file1 = token[idx];
            idx++;
            if (is_file(token[idx])) {
                single_ambiguous = AMBIG_BY_FILE;
                file2 = token[idx];
                rank2 = token[idx + 1];
            } else if (token[idx] == 'x') {
                single_ambiguous = AMBIG_BY_FILE;
                file2 = token[idx + 1];
                rank2 = token[idx + 2];
            } else {
                rank1 = token[idx];
                if (is_file(token[idx + 1])) {
                    double_ambiguous = 1;
                    file2 = token[idx + 1];
                    rank2 = token[idx + 2];
                } else if (token[idx + 1] == 'x') {
                    double_ambiguous = 1;
                    file2 = token[idx + 2];
                    rank2 = token[idx + 3];
                } else {
                    file2 = file1;
                    rank2 = rank1;
                }
            }
        } else if (token[idx] == 'x') {
            file2 = token[idx + 1];
            rank2 = token[idx + 2];
        } else {
            single_ambiguous = AMBIG_BY_RANK;
            rank1 = token[idx];
            if (token[idx + 1] == 'x') {
                file2 = token[idx + 2];
                rank2 = token[idx + 3];
            } else {
                file2 = token[idx + 1];
                rank2 = token[idx + 2];
            }
        }

        /* find fitting move */
        if (piece == 'N') {
            move = find_knight_move(board, file1, rank1, file2, rank2, single_ambiguous, double_ambiguous);
        } else if (piece == 'B') {
            move = find_bishop_move(board, file1, rank1, file2, rank2, single_ambiguous, double_ambiguous);
        } else if (piece == 'R') {
            move = find_rook_move(board, file1, rank1, file2, rank2, single_ambiguous, double_ambiguous);
        } else if (piece == 'Q') {
            move = find_queen_move(board, file1, rank1, file2, rank2, single_ambiguous, double_ambiguous);
        } else {
            move = find_king_move(board, file1, rank1, file2, rank2, single_ambiguous, double_ambiguous);
        }
    }
    /* if castle move */
    else if (token[0] == 'O') {
        /* assume kingside castle */
        int kingside = 1;

        /* if queen side castle though, change variable */
        if (token[3] == '-') {
            kingside = 0;
        }

        /* find fitting move */
        move = find_castle_move(board, kingside);
    }

    /* check if we found a move */
    if (!move) {
        fprintf(stderr, "THIS SHOULD NOT HAPPEN! MOVE %s SHOULD BE POSSIBLE!\n", token);
    }

    return move;
}

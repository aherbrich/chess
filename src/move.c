#include "../include/chess.h"
#include "../include/zobrist.h"


void move_piece(board_t* board, square_t from, square_t to) {
	bitboard_t mask = SQUARE_BB[from] | SQUARE_BB[to];
	board->piece_bb[board->playingfield[from]] ^= mask;
	board->piece_bb[board->playingfield[to]] &= ~mask;
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

void move_piece_quiet(board_t* board, square_t from, square_t to) {
	board->piece_bb[board->playingfield[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

/* Determines if two mves are the same */
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

/* Execute move */
int do_move(board_t *board, move_t *move) {
    /* FIRST: save old board state */
    HISTORY_HASHES[board->ply_no] = calculate_zobrist_hash(board);

    board->history[board->ply_no].castlerights = board->castle_rights;
    board->history[board->ply_no].captured = board->playingfield[move->to];
    board->history[board->ply_no].epsq = NO_SQUARE;
    board->history[board->ply_no].fifty_move_counter = board->fifty_move_counter;
    board->history[board->ply_no].full_move_counter = board->full_move_counter;

    bitboard_t from_mask = 1ULL << move->from;
    bitboard_t from_clear_mask = ~from_mask;
    bitboard_t to_mask = 1ULL << move->to;
    bitboard_t to_clear_mask = ~to_mask;

    /* SECONDLY: adjust counters */
    /* if move is a capture or a pawn move, reset counter */
    if ((move->flags & 0b0100) || (from_mask & board->piece_bb[W_PAWN]) ||
        (from_mask & board->piece_bb[B_PAWN])) {
        board->fifty_move_counter = 0;

    } else {
        /* else increase it */
        board->fifty_move_counter++;
    }

    /* if black is making the move/ made his move, then increase the full move
     * counter */
    if (board->player == BLACK) {
        board->full_move_counter++;
    }


    /* adjust castling rights */
    if(board->playingfield[move->from] == W_ROOK){
        if (move->from == 0) board->castle_rights &= ~(LONGSIDEW);
        if (move->from == 7) board->castle_rights &= ~(SHORTSIDEW);
    }
    else if(board->playingfield[move->from] == B_ROOK){
        if (move->from == 56) board->castle_rights &= ~(LONGSIDEB);
        if (move->from == 63) board->castle_rights &= ~(SHORTSIDEB);
    }
    else if(board->playingfield[move->from] == W_KING){
        board->castle_rights &= ~(SHORTSIDEW | LONGSIDEW);
    }
    else if(board->playingfield[move->from] == B_KING){
        board->castle_rights &= ~(SHORTSIDEB | LONGSIDEB);
    }

    if (move->flags & 0b0100) {
        /* first off, adjust castle rights if rooks were captured */
        if (move->to == 7) board->castle_rights &= ~(SHORTSIDEW);
        if (move->to == 0) board->castle_rights &= ~(LONGSIDEW);
        if (move->to == 63) board->castle_rights &= ~(SHORTSIDEB);
        if (move->to == 56) board->castle_rights &= ~(LONGSIDEB);
    }

    /* LASTLY: move execution
     * change the bitboard of moving piece
     * if moving piece is a... */

    bitboard_t mask = from_mask | to_mask;
    board->piece_bb[board->playingfield[move->from]] ^= mask;
    board->piece_bb[board->playingfield[move->to]] &= ~mask;
    board->playingfield[move->to] = board->playingfield[move->from];
    board->playingfield[move->from] = NO_PIECE;

    /*
     * EXIT EARLY STATEMENTS BEGIN
     */

    /* if move is quiet or simple capture */
    if (move->flags == QUIET || move->flags == CAPTURE) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is double pawn push */
    if (move->flags == DOUBLEP) {
        /* enpassant capture is possible next round */
        board->ep_possible = TRUE;
        if (board->player == WHITE) {
            board->ep_field = move->from + 8;
        } else {
            board->ep_field = move->from - 8;
        }

        board->history[board->ply_no].epsq = board->ep_field; 

        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is kingside castle */
    if (move->flags == KCASTLE) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if (board->player == WHITE) {
            mask = (1ULL << 7) | (1ULL << 5);
            board->piece_bb[W_ROOK] ^= mask;
            board->playingfield[5] = W_ROOK;
            board->playingfield[7] = NO_PIECE;

            board->castle_rights &= ~(SHORTSIDEW | LONGSIDEW);
        } else {
            mask = (1ULL << 63) | (1ULL << 61);
            board->piece_bb[B_ROOK] ^= mask;
            board->playingfield[61] = B_ROOK;
            board->playingfield[63] = NO_PIECE;

            board->castle_rights &= ~(SHORTSIDEB | LONGSIDEB);
        }

        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is queenside castle */
    if (move->flags == QCASTLE) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if (board->player == WHITE) {
            mask = (1ULL << 0) | (1ULL << 3);
            board->piece_bb[W_ROOK] ^= mask;
            board->playingfield[3] = W_ROOK;
            board->playingfield[0] = NO_PIECE;

            board->castle_rights &= ~(LONGSIDEW | SHORTSIDEW);
        } else {
            mask = (1ULL << 56) | (1ULL << 59);
            board->piece_bb[B_ROOK] ^= mask;
            board->playingfield[59] = B_ROOK;
            board->playingfield[56] = NO_PIECE;

            board->castle_rights &= ~(LONGSIDEB | SHORTSIDEB);
        }

        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is a queen promotion move */
    if (move->flags == QPROM || move->flags == QCPROM) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        /* remove pawn from to square and replace it by a queen */
        if (board->player == WHITE) {
            board->piece_bb[W_PAWN] &= to_clear_mask;
            board->piece_bb[W_QUEEN] |= to_mask;
            board->playingfield[move->to] = W_QUEEN;
        } else {
            board->piece_bb[B_PAWN] &= to_clear_mask;
            board->piece_bb[B_QUEEN] |= to_mask;
            board->playingfield[move->to] = B_QUEEN;
        }
        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is a rook promotion move */
    if (move->flags == RPROM || move->flags == RCPROM) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        /* remove pawn from to square and replace it by a rook */
        if (board->player == WHITE) {
            board->piece_bb[W_PAWN] &= to_clear_mask;
            board->piece_bb[W_ROOK] |= to_mask;
            board->playingfield[move->to] = W_ROOK;
        } else {
            board->piece_bb[B_PAWN] &= to_clear_mask;
            board->piece_bb[B_ROOK] |= to_mask;
            board->playingfield[move->to] = B_ROOK;
        }
        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is a knight promotion move */
    if (move->flags == KPROM || move->flags == KCPROM) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        /* remove pawn from to square and replace it by a knight */
        if (board->player == WHITE) {
            board->piece_bb[W_PAWN] &= to_clear_mask;
            board->piece_bb[W_KNIGHT] |= to_mask;
            board->playingfield[move->to] = W_KNIGHT;
        } else {
            board->piece_bb[B_PAWN] &= to_clear_mask;
            board->piece_bb[B_KNIGHT] |= to_mask;
            board->playingfield[move->to] = B_KNIGHT;
        }
        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is a bishop promotion move */
    if (move->flags == BPROM || move->flags == BCPROM) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        /* remove pawn from to square and replace it by a bishop */
        if (board->player == WHITE) {
            board->piece_bb[W_PAWN] &= to_clear_mask;
            board->piece_bb[W_BISHOP] |= to_mask;
            board->playingfield[move->to] = W_BISHOP;
        } else {
            board->piece_bb[B_PAWN] &= to_clear_mask;
            board->piece_bb[B_BISHOP] |= to_mask;
            board->playingfield[move->to] = B_BISHOP;
        }
        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    /* if move is an ep capture */
    if (move->flags == EPCAPTURE) {
        board->ep_possible = FALSE;
        board->ep_field = -1;

        /* special handling of removance of captured piece */
        if (board->player == WHITE) {
            board->history[board->ply_no].captured = B_PAWN;
            board->piece_bb[B_PAWN] &= ~SQUARE_BB[move->to - 8];
            board->playingfield[move->to-8] = NO_PIECE;
        } else {
            board->history[board->ply_no].captured = W_PAWN;
            board->piece_bb[W_PAWN] &= ~SQUARE_BB[move->to + 8];
            board->playingfield[move->to+8] = NO_PIECE;
        }

        board->ply_no++;
        board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
        /* dont forget to update the white, black and all bitboard */
        update_white_black_all_boards(board);
        return !is_in_check_after_move(board);
    }

    fprintf(stderr,
            "This should not happen, since all types of moves should have been "
            "checked exhaustively\n");
    exit(1);
    return 0;
}

/* Undos a move */
void undo_move(board_t *board) {
    /* reduce ply number */
    board->ply_no--;
    /* and recover old board state from board state saved for that exact ply */
    board_t *old_board = OLDSTATE[board->ply_no];
    recover_board(board, old_board);
}

/* Undos a move */
void undo_move_fast(board_t *board, move_t* move) {
    /* reduce ply number */
    board->ply_no--;
    board->castle_rights = board->history[board->ply_no].castlerights;
    board->ep_field = board->history[board->ply_no].epsq;
    board->ep_possible = (board->history[board->ply_no].epsq == NO_SQUARE) ? FALSE : TRUE;
    board->fifty_move_counter = board->history[board->ply_no].fifty_move_counter;
    board->full_move_counter = board->history[board->ply_no].full_move_counter;

    moveflags_t type = move->flags;
    square_t sq;

	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->to, move->from);
		break;
	case DOUBLEP:
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
		move_piece(board, move->to, move->from);
        if(board->player == WHITE){
            // then black made epcapture
            sq = move->to + 8;
            board->playingfield[sq] = W_PAWN;
            board->piece_bb[W_PAWN] |= SQUARE_BB[sq];
        } else{
            // then white made epcapture
            sq = move->to - 8;
            board->playingfield[sq] = B_PAWN;
            board->piece_bb[B_PAWN] |= SQUARE_BB[sq];
        }
		break;
	case KPROM:
	case BPROM:
	case RPROM:
	case QPROM:
        board->piece_bb[board->playingfield[move->to]] &= ~SQUARE_BB[move->to];
		board->playingfield[move->to] = NO_PIECE;
        if(board->player == WHITE){
            // then black promoted
            sq = move->from;
            board->playingfield[sq] = B_PAWN;
            board->piece_bb[B_PAWN] |= SQUARE_BB[sq];
        } else{
            // then white promoted
            sq = move->from;
            board->playingfield[sq] = W_PAWN;
            board->piece_bb[W_PAWN] |= SQUARE_BB[sq];
        }
		break;
	case KCPROM:
	case BCPROM:
	case RCPROM:
	case QCPROM:
		board->piece_bb[board->playingfield[move->to]] &= ~SQUARE_BB[move->to];
		board->playingfield[move->to] = NO_PIECE;
        if(board->player == WHITE){
            // then black promoted
            sq = move->from;
            board->playingfield[sq] = B_PAWN;
            board->piece_bb[B_PAWN] |= SQUARE_BB[sq];
        } else{
            // then white promoted
            sq = move->from;
            board->playingfield[sq] = W_PAWN;
            board->piece_bb[W_PAWN] |= SQUARE_BB[sq];
        }
        sq = move->to;
        board->playingfield[sq] = board->history[board->ply_no].captured;
        board->piece_bb[board->history[board->ply_no].captured] |= SQUARE_BB[sq];

		break;
	case CAPTURE:
		move_piece(board, move->to, move->from);
        sq = move->to;
        board->playingfield[sq] = board->history[board->ply_no].captured;
        board->piece_bb[board->history[board->ply_no].captured] |= SQUARE_BB[sq];
		break;
	}

	board->player = (board->player == BLACK) ? WHITE : BLACK;

    update_white_black_all_boards(board);
}
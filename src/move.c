#include "../include/chess.h"
#include "../include/zobrist.h"

void remove_piece(board_t* board, square_t sq){
    board->piece_bb[board->playingfield[sq]] &= ~SQUARE_BB[sq];
	board->playingfield[sq] = NO_PIECE;
}

void put_piece(board_t* board, piece_t pc, square_t sq){
    board->piece_bb[pc] |= SQUARE_BB[sq];
    board->playingfield[sq] = pc;
}


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
    HISTORY_HASHES[board->ply_no] = calculate_zobrist_hash(board);

    /* FIRST: save old board state */
    board->history[board->ply_no].castlerights = board->castle_rights;
    board->history[board->ply_no].captured = board->playingfield[move->to];
    board->history[board->ply_no].epsq = NO_SQUARE;
    board->history[board->ply_no].fifty_move_counter = board->fifty_move_counter;
    board->history[board->ply_no].full_move_counter = board->full_move_counter;


    /* SECONDLY: adjust counters */
    /* if move is a capture or a pawn move, reset counter */
    if ((move->flags & 0b0100) || (SQUARE_BB[move->from] & board->piece_bb[W_PAWN]) ||
        (SQUARE_BB[move->from] & board->piece_bb[B_PAWN])) {
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


    /*
     * EXIT EARLY STATEMENTS BEGIN
     */

    moveflags_t type = move->flags;

    board->ep_possible = FALSE;
    board->ep_field = -1;

	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->from, move->to);
		break;
	case DOUBLEP:
		move_piece_quiet(board, move->from, move->to);
			
        board->ep_possible = TRUE;
        if (board->player == WHITE) {
            board->ep_field = move->from + 8;
        } else {
            board->ep_field = move->from - 8;
        }

        board->history[board->ply_no].epsq = board->ep_field; 
		break;
	case KCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, g1);
			move_piece_quiet(board, h1, f1);
            board->castle_rights &= ~(SHORTSIDEW | LONGSIDEW);
		} else {
			move_piece_quiet(board, e8, g8);
			move_piece_quiet(board, h8, f8);
            board->castle_rights &= ~(SHORTSIDEB | LONGSIDEB);
		}			
		break;
	case QCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, c1); 
			move_piece_quiet(board, a1, d1);
            board->castle_rights &= ~(LONGSIDEW | SHORTSIDEW);
		} else {
			move_piece_quiet(board, e8, c8);
			move_piece_quiet(board, a8, d8);
            board->castle_rights &= ~(LONGSIDEB | SHORTSIDEB);
		}
		break;
	case EPCAPTURE:
		move_piece_quiet(board, move->from, move->to);

        if (board->player == WHITE) {
            board->history[board->ply_no].captured = B_PAWN;
            remove_piece(board, move->to - 8);
        } else {
            board->history[board->ply_no].captured = W_PAWN;
            remove_piece(board, move->to + 8);
        }
		break;
	case KPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_KNIGHT, move->to);
        } else {
            put_piece(board, B_KNIGHT, move->to);
        }
		break;
	case BPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_BISHOP, move->to);
        } else {
            put_piece(board, B_BISHOP, move->to);
        }
		break;
	case RPROM:
		remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_ROOK, move->to);
        } else {
            put_piece(board, B_ROOK, move->to);
        }
		break;
	case QPROM:
        remove_piece(board, move->from);
        if (board->player == WHITE) {
            put_piece(board, W_QUEEN, move->to);
        } else {
            put_piece(board, B_QUEEN, move->to);
        }
		break;
	case KCPROM:
        board->history[board->ply_no].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_KNIGHT, move->to);
        } else {
            put_piece(board, B_KNIGHT, move->to);
        }
		break;
	case BCPROM:
		board->history[board->ply_no].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_BISHOP, move->to);
        } else {
            put_piece(board, B_BISHOP, move->to);
        }
		break;
	case RCPROM:
		board->history[board->ply_no].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

       if (board->player == WHITE) {
            put_piece(board, W_ROOK, move->to);
        } else {
            put_piece(board, B_ROOK, move->to);
        }
		break;
	case QCPROM:
		board->history[board->ply_no].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_QUEEN, move->to);
        } else {
            put_piece(board, B_QUEEN, move->to);
        }
		break;
	case CAPTURE:
		board->history[board->ply_no].captured = board->playingfield[move->to];
		move_piece(board, move->from, move->to);
		break;
    default:
        fprintf(stderr,
            "This should not happen, since all types of moves should have been "
            "checked exhaustively\n");
        exit(1);
	}
    

    board->ply_no++;
    board->player = (board->player == WHITE) ? (BLACK) : (WHITE);
    update_white_black_all_boards(board);
    return !is_in_check_after_move(board);

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
            sq = move->to + 8;
            board->playingfield[sq] = W_PAWN;
            board->piece_bb[W_PAWN] |= SQUARE_BB[sq];
        } else{
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
            sq = move->from;
            board->playingfield[sq] = B_PAWN;
            board->piece_bb[B_PAWN] |= SQUARE_BB[sq];
        } else{
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
            sq = move->from;
            board->playingfield[sq] = B_PAWN;
            board->piece_bb[B_PAWN] |= SQUARE_BB[sq];
        } else{
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
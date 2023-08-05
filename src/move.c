#include "../include/chess.h"
#include "../include/zobrist.h"

const int CASTLE_FLAG_FROM[64] = {
    ~(LONGSIDEW), 15, 15, 15, ~(SHORTSIDEW | LONGSIDEW), 15, 15, ~(SHORTSIDEW),
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    ~(LONGSIDEB), 15, 15, 15, ~(SHORTSIDEB | LONGSIDEB), 15, 15, ~(SHORTSIDEB),
};

const int CASTLE_FLAG_TO[64] = {
    ~(LONGSIDEW), 15, 15, 15, 15, 15, 15, ~(SHORTSIDEW),
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    ~(LONGSIDEB), 15, 15, 15, 15, 15, 15, ~(SHORTSIDEB),
};

void remove_piece(board_t* board, square_t sq){
    board->hash ^= zobrist_table.piece_random64[board->playingfield[sq]][sq];
    board->piece_bb[board->playingfield[sq]] &= ~SQUARE_BB[sq];
	board->playingfield[sq] = NO_PIECE;
}

void put_piece(board_t* board, piece_t pc, square_t sq){
    board->piece_bb[pc] |= SQUARE_BB[sq];
    board->playingfield[sq] = pc;
    board->hash ^= zobrist_table.piece_random64[pc][sq];
}


void move_piece(board_t* board, square_t from, square_t to) {
    if(board->playingfield[to] == NO_PIECE) exit(3);
    board->hash ^= zobrist_table.piece_random64[board->playingfield[from]][from] ^ 
                    zobrist_table.piece_random64[board->playingfield[from]][to] ^
                    zobrist_table.piece_random64[board->playingfield[to]][to];
	bitboard_t mask = SQUARE_BB[from] | SQUARE_BB[to];
	board->piece_bb[board->playingfield[from]] ^= mask;
	board->piece_bb[board->playingfield[to]] &= ~mask;
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

void move_piece_quiet(board_t* board, square_t from, square_t to) {
    board->hash ^= zobrist_table.piece_random64[board->playingfield[from]][from] ^ 
                    zobrist_table.piece_random64[board->playingfield[from]][to];
	board->piece_bb[board->playingfield[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
	board->playingfield[to] = board->playingfield[from];
	board->playingfield[from] = NO_PIECE;
}

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

/* Checks if piece would capture on a given TO-field and returns the piece type
 */
int is_capture(bitboard_t to, board_t *board) {
    bitboard_t to_mask = (1ULL << to);
    if(board->player == WHITE){
        if (board->piece_bb[B_PAWN] & to_mask) return PAWN_ID;
        if (board->piece_bb[B_KNIGHT] & to_mask) return KNIGHT_ID;
        if (board->piece_bb[B_BISHOP] & to_mask) return BISHOP_ID;
        if (board->piece_bb[B_ROOK] & to_mask) return ROOK_ID;
        if (board->piece_bb[B_QUEEN] & to_mask) return QUEEN_ID;
        if (board->piece_bb[B_KING] & to_mask) return KING_ID;
    } else{
        if (board->piece_bb[W_PAWN] & to_mask) return PAWN_ID;
        if (board->piece_bb[W_KNIGHT] & to_mask) return KNIGHT_ID;
        if (board->piece_bb[W_BISHOP] & to_mask) return BISHOP_ID;
        if (board->piece_bb[W_ROOK] & to_mask) return ROOK_ID;
        if (board->piece_bb[W_QUEEN] & to_mask) return QUEEN_ID;
        if (board->piece_bb[W_KING] & to_mask) return KING_ID;
    }
    return EMPTY;
}

/* Checks if a player is in check WHEN AT TURN */
/* WARNING: Only call AFTER move generation (and BEFORE making move)
(since we determine if we are in check in move generation anyway,
but would switch players if we made move early) */
int is_in_check(board_t *board) {
	return(board->checkers);
}


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
void do_move(board_t *board, move_t *move) {
    /* save current board hash in array */
    HISTORY_HASHES[board->ply_no] = board->hash;
    
    /* increase board ply number */
    board->ply_no++;
    uint16_t ply = board->ply_no;

    /* reset history fields for board at new ply */
    board->history[ply].captured = NO_PIECE;
    board->history[ply].epsq = NO_SQUARE;
    board->history[ply].castlerights = board->history[ply-1].castlerights;
    board->history[ply].fifty_move_counter = board->history[ply-1].fifty_move_counter;
    board->history[ply].full_move_counter = board->history[ply-1].full_move_counter;

    /* adjustment of zobrist hash */
    /* xor out the (old) ep square if an ep sqaure was given i.e. epcapture was possible at ply-1*/
    if(board->history[ply-1].epsq != NO_SQUARE) board->hash ^= zobrist_table.flag_random64[board->history[ply-1].epsq % 8];
    /* xor out the (old) castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[ply-1].castlerights+8];

    /* reset fifty-counter if move is a capture or a pawn move*/
    if ((move->flags & 0b0100) || (SQUARE_BB[move->from] & board->piece_bb[W_PAWN]) ||
        (SQUARE_BB[move->from] & board->piece_bb[B_PAWN])) {
        board->history[ply].fifty_move_counter = 0;

    } else {
        /* else increase it */
        board->history[ply].fifty_move_counter++;
    }

    /* if black is making the move/ made his move, then increase the full move
     * counter */
    if (board->player == BLACK) {
        board->history[ply].full_move_counter++;
    }

    /* adjust castling rights if (potentially) king or rook moved from their start squares */
    if (move->from == a1) board->history[ply].castlerights &= ~(LONGSIDEW);
    else if (move->from == h1) board->history[ply].castlerights &= ~(SHORTSIDEW);
    else if (move->from == a8) board->history[ply].castlerights &= ~(LONGSIDEB);
    else if (move->from == h8) board->history[ply].castlerights &= ~(SHORTSIDEB);
    else if(move->from == e1) board->history[ply].castlerights &= ~(SHORTSIDEW | LONGSIDEW);
    else if(move->from == e8) board->history[ply].castlerights &= ~(SHORTSIDEB | LONGSIDEB);

    

    /* adjust castle rights if rooks were (potentially) captured on their start squares */
    if (move->to == h1) board->history[ply].castlerights &= ~(SHORTSIDEW);
    else if (move->to == a1) board->history[ply].castlerights &= ~(LONGSIDEW);
    else if (move->to == h8) board->history[ply].castlerights &= ~(SHORTSIDEB);
    else if (move->to == a8) board->history[ply].castlerights &= ~(LONGSIDEB);


    moveflags_t type = move->flags;
	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->from, move->to);
		break;
	case DOUBLEP:
		move_piece_quiet(board, move->from, move->to);
        if (board->player == WHITE) {
            board->history[ply].epsq = move->from + 8;
        } else {
            board->history[ply].epsq = move->from - 8;
        }
        /* xor in the new ep square */
        board->hash ^= zobrist_table.flag_random64[board->history[ply].epsq % 8];
		break;
	case KCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, g1);
			move_piece_quiet(board, h1, f1);
            board->history[ply].castlerights &= ~(SHORTSIDEW | LONGSIDEW);
		} else {
			move_piece_quiet(board, e8, g8);
			move_piece_quiet(board, h8, f8);
            board->history[ply].castlerights &= ~(SHORTSIDEB | LONGSIDEB);
		}			
		break;
	case QCASTLE:
		if (board->player == WHITE) {
			move_piece_quiet(board, e1, c1); 
			move_piece_quiet(board, a1, d1);
            board->history[ply].castlerights &= ~(LONGSIDEW | SHORTSIDEW);
		} else {
			move_piece_quiet(board, e8, c8);
			move_piece_quiet(board, a8, d8);
            board->history[ply].castlerights &= ~(LONGSIDEB | SHORTSIDEB);
		}
		break;
	case EPCAPTURE:
		move_piece_quiet(board, move->from, move->to);

        if (board->player == WHITE) {
            board->history[ply-1].captured = B_PAWN;
            remove_piece(board, move->to - 8);
        } else {
            board->history[ply-1].captured = W_PAWN;
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
        board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_KNIGHT, move->to);
        } else {
            put_piece(board, B_KNIGHT, move->to);
        }
		break;
	case BCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_BISHOP, move->to);
        } else {
            put_piece(board, B_BISHOP, move->to);
        }
		break;
	case RCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

       if (board->player == WHITE) {
            put_piece(board, W_ROOK, move->to);
        } else {
            put_piece(board, B_ROOK, move->to);
        }
		break;
	case QCPROM:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];

        remove_piece(board, move->from);
        remove_piece(board, move->to);

        if (board->player == WHITE) {
            put_piece(board, W_QUEEN, move->to);
        } else {
            put_piece(board, B_QUEEN, move->to);
        }
		break;
	case CAPTURE:
		board->history[board->ply_no-1].captured = board->playingfield[move->to];
		move_piece(board, move->from, move->to);
		break;
    default:
        fprintf(stderr,
            "This should not happen, since all types of moves should have been "
            "checked exhaustively\n");
        exit(1);
	}
    
    board->player = SWITCHSIDES(board->player);
    /* xor out old player and xor in the new player */
    board->hash ^= zobrist_table.flag_random64[24] ^ zobrist_table.flag_random64[25];
    /* xor in the new castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[ply].castlerights+8];
}

/* Undos a move */
void undo_move(board_t *board, move_t* move) {
    /* xor out old castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].castlerights+8];
    /* reduce ply number */
    board->ply_no--;
    /* xor in new castle rights */
    board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].castlerights+8];
    /* xor out the (old) ep square if an ep sqaure was given i.e. epcapture was possible at ply+1*/
    if(board->history[board->ply_no].epsq != NO_SQUARE) board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no].epsq % 8];

    moveflags_t type = move->flags;

	switch (type) {
	case QUIET:
		move_piece_quiet(board, move->to, move->from);
		break;
	case DOUBLEP:
    /* xor in new ep square */
        board->hash ^= zobrist_table.flag_random64[board->history[board->ply_no+1].epsq % 8];
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
		move_piece_quiet(board, move->to, move->from);
        if(board->player == WHITE){
            put_piece(board, W_PAWN, move->to + 8);
        } else{
            put_piece(board, B_PAWN, move->to - 8);
        }
		break;
	case KPROM:
	case BPROM:
	case RPROM:
	case QPROM:
        remove_piece(board, move->to);
        if(board->player == WHITE){
            put_piece(board, B_PAWN, move->from);
        } else{
            put_piece(board, W_PAWN, move->from);
        }
		break;
	case KCPROM:
	case BCPROM:
	case RCPROM:
	case QCPROM:
        remove_piece(board, move->to);
        if(board->player == WHITE){
            put_piece(board, B_PAWN, move->from);
        } else{
            put_piece(board, W_PAWN, move->from);
        }
        put_piece(board, board->history[board->ply_no].captured, move->to);
		break;
	case CAPTURE:
		move_piece_quiet(board, move->to, move->from);
        put_piece(board, board->history[board->ply_no].captured, move->to);
		break;
	}

	board->player = SWITCHSIDES(board->player);
    /* xor in the new castle rights */
    board->hash ^= zobrist_table.flag_random64[24] ^ zobrist_table.flag_random64[25];
}
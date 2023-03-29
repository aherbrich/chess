#include "../include/chess.h"
#include "../include/magic.h"

bitboard_t MASK_FILE[8];
bitboard_t MASK_RANK[8];
bitboard_t CLEAR_FILE[8];
bitboard_t CLEAR_RANK[8];
bitboard_t UNIBOARD;

bitboard_t ROOK_ATTACK[64][4096];
bitboard_t BISHOP_ATTACK[64][4096];
bitboard_t KNIGHT_ATTACK[64]; 
bitboard_t KING_ATTACK[64];

int is_capture(bitboard_t to, board_t *board){
    bitboard_t to_mask = (1ULL << to);
    if(board->player == WHITE){
        if(board->black & to_mask){
            if(board->blackpawns & to_mask) return PAWN;
            if(board->blackknights & to_mask) return KNIGHT;
            if(board->blackbishops & to_mask) return BISHOP;
            if(board->blackrooks & to_mask) return ROOK;
            if(board->blackqueens & to_mask) return QUEEN;
            if(board->blackking & to_mask) return KING;
        }
        return 0;
    } else{
        if(board->white & to_mask){
            if(board->whitepawns & to_mask) return PAWN;
            if(board->whiteknights & to_mask) return KNIGHT;
            if(board->whitebishops & to_mask) return BISHOP;
            if(board->whiterooks & to_mask) return ROOK;
            if(board->whitequeens & to_mask) return QUEEN;
            if(board->whiteking & to_mask) return KING;
        }
        return 0;
    }
}

int is_in_check_after_move(board_t *board){
    if(board->player == BLACK){
        if(!board->whiteking) return TRUE;
        int king_sq = find_1st_bit(board->whiteking);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */ 
        attackers = KNIGHT_ATTACK[king_sq] & board->blackknights;
        if(attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = rook_mask(king_sq) & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->blackrooks;
        if(attackers) return TRUE;

        blockers = bishop_mask(king_sq) & board->all;
        int k = transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->blackbishops;
        if(attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) & board->blackqueens;
        if(attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->whiteking & CLEAR_FILE[A]) << 7) | ((board->whiteking & CLEAR_FILE[H]) << 9)) & board->blackpawns;
        if(attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->blackking;
        if(attackers) return TRUE;

        return FALSE;
    } else{
        if(!board->blackking) return TRUE;
        int king_sq = find_1st_bit(board->blackking);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */ 
        attackers = KNIGHT_ATTACK[king_sq] & board->whiteknights;
        if(attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = rook_mask(king_sq) & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->whiterooks;
        if(attackers) return TRUE;

        blockers = bishop_mask(king_sq) & board->all;
        int k = transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->whitebishops;
        if(attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) & board->whitequeens;
        if(attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->blackking & CLEAR_FILE[H]) >> 7) | ((board->blackking & CLEAR_FILE[A]) >> 9)) & board->whitepawns;
        if(attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->whiteking;
        if(attackers) return TRUE;

        return FALSE;
    }
}

int is_in_check(board_t *board){
    if(board->player == WHITE){
        if(!board->whiteking) return TRUE;
        int king_sq = find_1st_bit(board->whiteking);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */ 
        attackers = KNIGHT_ATTACK[king_sq] & board->blackknights;
        if(attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = rook_mask(king_sq) & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->blackrooks;
        if(attackers) return TRUE;

        blockers = bishop_mask(king_sq) & board->all;
        int k = transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->blackbishops;
        if(attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) & board->blackqueens;
        if(attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->whiteking & CLEAR_FILE[A]) << 7) | ((board->whiteking & CLEAR_FILE[H]) << 9)) & board->blackpawns;
        if(attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->blackking;
        if(attackers) return TRUE;

        return FALSE;
    } else{
        if(!board->blackking) return TRUE;
        int king_sq = find_1st_bit(board->blackking);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */ 
        attackers = KNIGHT_ATTACK[king_sq] & board->whiteknights;
        if(attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = rook_mask(king_sq) & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->whiterooks;
        if(attackers) return TRUE;

        blockers = bishop_mask(king_sq) & board->all;
        int k = transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->whitebishops;
        if(attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) & board->whitequeens;
        if(attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->blackking & CLEAR_FILE[H]) >> 7) | ((board->blackking & CLEAR_FILE[A]) >> 9)) & board->whitepawns;
        if(attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->whiteking;
        if(attackers) return TRUE;

        return FALSE;
    }
}

inline bitboard_t get_knight_attacks(int sq, board_t *board){
    bitboard_t attacks = KNIGHT_ATTACK[sq];
    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);
    return attacks;
}

inline bitboard_t get_king_attacks(int sq, board_t *board){
    bitboard_t attacks = KING_ATTACK[sq];
    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);
    return attacks;
}

inline bitboard_t get_white_pawn_attacks(bitboard_t pawn, board_t *board){
	bitboard_t pawn_one_step = (pawn << 8) & ~board->all; 
	bitboard_t pawn_two_steps = ((pawn_one_step & MASK_RANK[RANK3]) << 8) & ~board->all; 
	bitboard_t pawn_valid_moves = pawn_one_step | pawn_two_steps;

	bitboard_t pawn_left_attack = (pawn & CLEAR_FILE[A]) << 7;
	bitboard_t pawn_right_attack = (pawn & CLEAR_FILE[H]) << 9;
	bitboard_t pawn_attacks = pawn_left_attack | pawn_right_attack;
	bitboard_t pawn_valid_attacks = pawn_attacks & board->black;

	bitboard_t pawn_all_valid = pawn_valid_moves | pawn_valid_attacks;

	return pawn_all_valid;
}

inline bitboard_t get_black_pawn_attacks(bitboard_t pawn, board_t *board){
	bitboard_t pawn_one_step = (pawn >> 8) & ~board->all; 
	bitboard_t pawn_two_steps = ((pawn_one_step & MASK_RANK[RANK6]) >> 8) & ~board->all; 
	bitboard_t pawn_valid_moves = pawn_one_step | pawn_two_steps;

	bitboard_t pawn_left_attack = (pawn & CLEAR_FILE[A]) >> 9;
	bitboard_t pawn_right_attack = (pawn & CLEAR_FILE[H]) >> 7;
	bitboard_t pawn_attacks = pawn_left_attack | pawn_right_attack;
	bitboard_t pawn_valid_attacks = pawn_attacks & board->white;

	bitboard_t pawn_all_valid = pawn_valid_moves | pawn_valid_attacks;

	return pawn_all_valid;
}

inline bitboard_t get_bishop_attacks(int sq, board_t *board){
    bitboard_t blockers = bishop_mask(sq) & board->all;
    int j = transform(blockers, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
    bitboard_t attacks = BISHOP_ATTACK[sq][j];

    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);

    return(attacks);
}

inline bitboard_t get_rook_attacks(int sq, board_t *board){
    bitboard_t blockers = rook_mask(sq) & board->all;
    int j = transform(blockers, ROOK_MAGIC[sq], ROOK_BITS[sq]);
    bitboard_t attacks = ROOK_ATTACK[sq][j];

    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);

    return(attacks);
}

void generate_knight_moves(bitboard_t knights, board_t *board, list_t *movelst){
    while(knights){
        idx_t from = pop_1st_bit(&knights);
        bitboard_t attacks = get_knight_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            int captured_piece;
            if((captured_piece = is_capture(to, board))){
                move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - KNIGHT));
            }
            else{
                move = generate_move(from, to, QUIET, 0);
            }
            push(movelst, move);
        }
    }
}

void generate_king_moves(bitboard_t kings, board_t *board, list_t *movelst){
    idx_t from = pop_1st_bit(&kings);
    bitboard_t attacks = get_king_attacks(from, board);

    while(attacks){
        idx_t to = pop_1st_bit(&attacks);
        move_t *move;
        int captured_piece;
        if((captured_piece = is_capture(to, board))){
            move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - KING));
        }
        else{
            move = generate_move(from, to, QUIET, 0);
        }
        push(movelst, move);
    }
}

void generate_bishop_moves(bitboard_t bishops, board_t *board, list_t *movelst, flag_t actually_queen){
    while(bishops){
        idx_t from = pop_1st_bit(&bishops);
        bitboard_t attacks = get_bishop_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            int captured_piece;
            if((captured_piece = is_capture(to, board))){
                if(actually_queen){
                    move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - QUEEN));
                } else{
                    move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - BISHOP));
                }
            }
            else{
                move = generate_move(from, to, QUIET, 0);
            }
            push(movelst, move);
        }
    }
}

void generate_rook_moves(bitboard_t rooks, board_t *board, list_t *movelst, flag_t actually_queen){
    while(rooks){
        idx_t from = pop_1st_bit(&rooks);
        bitboard_t attacks = get_rook_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            int captured_piece;
            if((captured_piece = is_capture(to, board))){
                if(actually_queen){
                    move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - QUEEN));
                } else{
                    move = generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - ROOK));
                }
            }
            else{
                move = generate_move(from, to, QUIET, 0);
            }
            push(movelst, move);
        }
    }
}

void generate_queen_moves(bitboard_t queens, board_t *board, list_t *movelst){
    generate_bishop_moves(queens, board, movelst, TRUE);
    generate_rook_moves(queens, board, movelst, TRUE);
}

void generate_white_pawn_moves(board_t *board, list_t *movelst){
    bitboard_t pawns = board->whitepawns;
    while(pawns){
        idx_t from = pop_1st_bit(&pawns);
        bitboard_t piece = (1ULL << from);
        bitboard_t attacks = get_white_pawn_attacks(piece, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            /* if move  is a promotion move */
            if(to >= 56 && to <= 63){
                if(is_capture(to, board)){
                    push(movelst, generate_move(from, to, KCPROM, 2500));
                    push(movelst, generate_move(from, to, BCPROM, 2510));
                    push(movelst, generate_move(from, to, RCPROM, 2520));
                    push(movelst, generate_move(from, to, QCPROM, 2530));
                }
                else{
                    push(movelst, generate_move(from, to, KPROM, 2460));
                    push(movelst, generate_move(from, to, BPROM, 2470));
                    push(movelst, generate_move(from, to, RPROM, 2480));
                    push(movelst, generate_move(from, to, QPROM, 2490));
                }
            }
            /* if move is a double pawn push */ 
            else if((to - 16) == from){
                push(movelst, generate_move(from, to, DOUBLEP, 0));
            }
            /* if move is a quiet or capture move */
            else{
                int captured_piece;
                if((captured_piece = is_capture(to, board))){
                    push(movelst, generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - PAWN)));
                }
                else{
                    push(movelst, generate_move(from, to, QUIET, 0));
                }
            }
        }
    }
}

void generate_black_pawn_moves(board_t *board, list_t *movelst){
    bitboard_t pawns = board->blackpawns;
    while(pawns){
        idx_t from = pop_1st_bit(&pawns);
        bitboard_t piece = (1ULL << from);
        bitboard_t attacks = get_black_pawn_attacks(piece, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            /* if move  is a promotion move */
            if(to >= 0 && to <= 7){
                if(is_capture(to, board)){
                    push(movelst, generate_move(from, to, KCPROM, 2500));
                    push(movelst, generate_move(from, to, BCPROM, 2510));
                    push(movelst, generate_move(from, to, RCPROM, 2510));
                    push(movelst, generate_move(from, to, QCPROM, 2530));
                }
                else{
                    push(movelst, generate_move(from, to, KPROM, 2460));
                    push(movelst, generate_move(from, to, BPROM, 2470));
                    push(movelst, generate_move(from, to, RPROM, 2480));
                    push(movelst, generate_move(from, to, QPROM, 2490));
                }
            }
            /* if move is a double pawn push */ 
            else if((to + 16) == from){
                push(movelst, generate_move(from, to, DOUBLEP, 0));
            }
            /* if move is a quiet or capture move */
            else{
                int captured_piece;
                if((captured_piece = is_capture(to, board))){
                    push(movelst, generate_move(from, to, CAPTURE, captured_piece * 100 + (KING - PAWN)));
                }
                else{
                    push(movelst, generate_move(from, to, QUIET, 0));
                }
            }
        }
    }
}

void generate_white_enpassant_moves(board_t *board, list_t *movelst){
    bitboard_t pawns = board->whitepawns;
    if(board->ep_possible){
        bitboard_t ep = (1ULL << board->ep_field);
        while(pawns){
            idx_t from = pop_1st_bit(&pawns);
            bitboard_t piece = (1ULL << from);
            bitboard_t attacks = (((piece & CLEAR_FILE[A]) << 7) & ep) | (((piece & CLEAR_FILE[H]) << 9) & ep);

            while(attacks){
                idx_t to = pop_1st_bit(&attacks);
                push(movelst, generate_move(from, to, EPCAPTURE, PAWN * 100 + (KING - PAWN)));
            }
        }
    }

    return;
} 

void generate_black_enpassant_moves(board_t *board, list_t *movelst){
    bitboard_t pawns = board->blackpawns;
    if(board->ep_possible){
        bitboard_t ep = (1ULL << board->ep_field);
        while(pawns){
            idx_t from = pop_1st_bit(&pawns);
            bitboard_t piece = (1ULL << from);
            bitboard_t attacks = (((piece & CLEAR_FILE[H]) >> 7) & ep) | (((piece & CLEAR_FILE[A]) >> 9) & ep);

            while(attacks){
                idx_t to = pop_1st_bit(&attacks);
                push(movelst, generate_move(from, to, EPCAPTURE, PAWN * 100 + (KING - PAWN)));
            }
        }
    }
    
    return;
} 

void generate_white_castle_moves(board_t *board, list_t *movelst){
    if(is_in_check(board)) return;
    /* king/shortside */
    if((board->all & ((1ULL << 5) | (1ULL << 6))) == 0 && (board->castle_rights & SHORTSIDEW)){
        board->whiteking = ((1ULL << 5));
        int through_check = is_in_check(board);
        board->whiteking = ((1ULL << 4));
        if(!through_check) push(movelst, generate_move(4, 6, KCASTLE, 0));
        
    }
    /* queen/longside */
    if((board->all & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) == 0 && (board->castle_rights & LONGSIDEW)){
        board->whiteking = ((1ULL << 3));
        int through_check = is_in_check(board);
        board->whiteking = ((1ULL << 4));
        if(!through_check) push(movelst, generate_move(4, 2, QCASTLE, 0));
        
    }
} 

void generate_black_castle_moves(board_t *board, list_t *movelst){
    if(is_in_check(board)) return;
    /* king/shortside */
    if((board->all & ((1ULL << 61) | (1ULL << 62))) == 0 && (board->castle_rights & SHORTSIDEB)){
        board->blackking = ((1ULL << 61));
        int through_check = is_in_check(board);
        board->blackking = ((1ULL << 60));
        if(!through_check) push(movelst, generate_move(60, 62, KCASTLE, 0));;
        
    }
    /* queen/longside */
    if((board->all & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) == 0 && (board->castle_rights & LONGSIDEB)){
        board->blackking = ((1ULL << 59));
        int through_check = is_in_check(board);
        board->blackking = ((1ULL << 60));
        if(!through_check) push(movelst, generate_move(60, 58, QCASTLE, 0));;
        
    }
} 

/* Generate pseudo legal moves */
list_t* generate_pseudo_moves(board_t *board){
    list_t *movelst = new_list();

    if(board->player == WHITE){
        generate_white_pawn_moves(board, movelst);
        generate_knight_moves(board->whiteknights, board, movelst);
        generate_bishop_moves(board->whitebishops, board, movelst, FALSE);
        generate_rook_moves(board->whiterooks, board, movelst, FALSE);
        generate_queen_moves(board->whitequeens, board, movelst);
        generate_king_moves(board->whiteking, board, movelst);
        generate_white_enpassant_moves(board, movelst);
        generate_white_castle_moves(board, movelst);
    } else{
        generate_black_pawn_moves(board, movelst);
        generate_knight_moves(board->blackknights, board, movelst);
        generate_bishop_moves(board->blackbishops, board, movelst, FALSE);
        generate_rook_moves(board->blackrooks, board, movelst, FALSE);
        generate_queen_moves(board->blackqueens, board, movelst);
        generate_king_moves(board->blackking, board, movelst);
        generate_black_enpassant_moves(board, movelst);
        generate_black_castle_moves(board, movelst);
    }

    return movelst;
}

/* Filters out illegal moves from pseudo legal move list */
list_t* filter_illegal_moves(board_t* board, list_t *movelst){
    list_t* legalmoves = new_list();

    while(movelst->len != 0){
        move_t *move = pop(movelst);
        do_move(board, move);
        int in_check = is_in_check_after_move(board);
        if(!in_check) push(legalmoves, move);
        undo_move(board);
        if(in_check) free(move);
    }
    free(movelst);
    return(legalmoves);
}

/* Generate legal moves */
list_t* generate_moves(board_t* board){
    list_t *legal = filter_illegal_moves(board, generate_pseudo_moves(board));
    return legal;
}

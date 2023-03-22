#include "../include/chess.h"

bitboard_t MASK_FILE[8];
bitboard_t MASK_RANK[8];
bitboard_t CLEAR_FILE[8];
bitboard_t CLEAR_RANK[8];
bitboard_t UNIBOARD;

const uint64_t ROOK_MAGIC[64] = {
  0xa8002c000108020ULL,
  0x4440200140003000ULL,
  0x8080200010011880ULL,
  0x380180080141000ULL,
  0x1a00060008211044ULL,
  0x410001000a0c0008ULL,
  0x9500060004008100ULL,
  0x100024284a20700ULL,
  0x802140008000ULL,
  0x80c01002a00840ULL,
  0x402004282011020ULL,
  0x9862000820420050ULL,
  0x1001448011100ULL,
  0x6432800200800400ULL,
  0x40100010002000cULL,
  0x2800d0010c080ULL,
  0x90c0008000803042ULL,
  0x4010004000200041ULL,
  0x3010010200040ULL,
  0xa40828028001000ULL,
  0x123010008000430ULL,
  0x24008004020080ULL,
  0x60040001104802ULL,
  0x582200028400d1ULL,
  0x4000802080044000ULL,
  0x408208200420308ULL,
  0x610038080102000ULL,
  0x3601000900100020ULL,
  0x80080040180ULL,
  0xc2020080040080ULL,
  0x80084400100102ULL,
  0x4022408200014401ULL,
  0x40052040800082ULL,
  0xb08200280804000ULL,
  0x8a80a008801000ULL,
  0x4000480080801000ULL,
  0x911808800801401ULL,
  0x822a003002001894ULL,
  0x401068091400108aULL,
  0x4a10a00004cULL,
  0x2000800640008024ULL,
  0x1486408102020020ULL,
  0x100a000d50041ULL,
  0x810050020b0020ULL,
  0x204000800808004ULL,
  0x20048100a000cULL,
  0x112000831020004ULL,
  0x9000040810002ULL,
  0x440490200208200ULL,
  0x8910401000200040ULL,
  0x6404200050008480ULL,
  0x4b824a2010010100ULL,
  0x4080801810c0080ULL,
  0x400802a0080ULL,
  0x8224080110026400ULL,
  0x40002c4104088200ULL,
  0x1002100104a0282ULL,
  0x1208400811048021ULL,
  0x3201014a40d02001ULL,
  0x5100019200501ULL,
  0x101000208001005ULL,
  0x2008450080702ULL,
  0x1002080301d00cULL,
  0x410201ce5c030092ULL,
};
const uint64_t BISHOP_MAGIC[64] = {
  0x40210414004040ULL,
  0x2290100115012200ULL,
  0xa240400a6004201ULL,
  0x80a0420800480ULL,
  0x4022021000000061ULL,
  0x31012010200000ULL,
  0x4404421051080068ULL,
  0x1040882015000ULL,
  0x8048c01206021210ULL,
  0x222091024088820ULL,
  0x4328110102020200ULL,
  0x901cc41052000d0ULL,
  0xa828c20210000200ULL,
  0x308419004a004e0ULL,
  0x4000840404860881ULL,
  0x800008424020680ULL,
  0x28100040100204a1ULL,
  0x82001002080510ULL,
  0x9008103000204010ULL,
  0x141820040c00b000ULL,
  0x81010090402022ULL,
  0x14400480602000ULL,
  0x8a008048443c00ULL,
  0x280202060220ULL,
  0x3520100860841100ULL,
  0x9810083c02080100ULL,
  0x41003000620c0140ULL,
  0x6100400104010a0ULL,
  0x20840000802008ULL,
  0x40050a010900a080ULL,
  0x818404001041602ULL,
  0x8040604006010400ULL,
  0x1028044001041800ULL,
  0x80b00828108200ULL,
  0xc000280c04080220ULL,
  0x3010020080880081ULL,
  0x10004c0400004100ULL,
  0x3010020200002080ULL,
  0x202304019004020aULL,
  0x4208a0000e110ULL,
  0x108018410006000ULL,
  0x202210120440800ULL,
  0x100850c828001000ULL,
  0x1401024204800800ULL,
  0x41028800402ULL,
  0x20642300480600ULL,
  0x20410200800202ULL,
  0xca02480845000080ULL,
  0x140c404a0080410ULL,
  0x2180a40108884441ULL,
  0x4410420104980302ULL,
  0x1108040046080000ULL,
  0x8141029012020008ULL,
  0x894081818082800ULL,
  0x40020404628000ULL,
  0x804100c010c2122ULL,
  0x8168210510101200ULL,
  0x1088148121080ULL,
  0x204010100c11010ULL,
  0x1814102013841400ULL,
  0xc00010020602ULL,
  0x1045220c040820ULL,
  0x12400808070840ULL,
  0x2004012a040132ULL,
};

bitboard_t ROOK_ATTACK[64][4096];
bitboard_t BISHOP_ATTACK[64][4096];
bitboard_t KNIGHT_ATTACK[64]; 
bitboard_t KING_ATTACK[64];

int is_capture(bitboard_t to, board_t *board){
    if(board->player == WHITE){
        if(board->black & (1ULL << to)){
            return 1;
        }
        return 0;
    } else{
        if(board->white & (1ULL << to)){
            return 1;
        }
        return 0;
    }
}

bitboard_t get_knight_attacks(int sq, board_t *board){
    bitboard_t attacks = KNIGHT_ATTACK[sq];
    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);
    return attacks;
}

bitboard_t get_king_attacks(int sq, board_t *board){
    bitboard_t attacks = KING_ATTACK[sq];
    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);
    return attacks;
}

bitboard_t get_white_pawn_attacks(bitboard_t pawn, board_t *board){
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

bitboard_t get_black_pawn_attacks(bitboard_t pawn, board_t *board){
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

bitboard_t get_bishop_attacks(int sq, board_t *board){
    bitboard_t blockers = bishop_mask(sq) & board->all;
    int j = transform(blockers, BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
    bitboard_t attacks = BISHOP_ATTACK[sq][j];

    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);

    return(attacks);
}

bitboard_t get_rook_attacks(int sq, board_t *board){
    bitboard_t blockers = rook_mask(sq) & board->all;
    int j = transform(blockers, ROOK_MAGIC[sq], ROOK_BITS[sq]);
    bitboard_t attacks = ROOK_ATTACK[sq][j];

    attacks = (board->player == WHITE)?(attacks&~board->white):(attacks&~board->black);

    return(attacks);
}

/* DONE */
void generate_knight_moves(bitboard_t knights, board_t *board, node_t *movelst){
    while(knights){
        idx_t from = pop_1st_bit(&knights);
        bitboard_t attacks = get_knight_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            if(is_capture(to, board)){
                move = generate_move(from, to, CAPTURE);
            }
            else{
                move = generate_move(from, to, QUIET);
            }
            add(movelst, move);
        }
    }
}

/* DONE */
void generate_king_moves(bitboard_t kings, board_t *board, node_t *movelst){
    idx_t from = pop_1st_bit(&kings);
    bitboard_t attacks = get_king_attacks(from, board);

    while(attacks){
        idx_t to = pop_1st_bit(&attacks);
        move_t *move;
        if(is_capture(to, board)){
            move = generate_move(from, to, CAPTURE);
        }
        else{
            move = generate_move(from, to, QUIET);
        }
        add(movelst, move);
    }
}

/* DONE */
void generate_bishop_moves(bitboard_t bishops, board_t *board, node_t *movelst){
    while(bishops){
        idx_t from = pop_1st_bit(&bishops);
        bitboard_t attacks = get_bishop_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            if(is_capture(to, board)){
                move = generate_move(from, to, CAPTURE);
            }
            else{
                move = generate_move(from, to, QUIET);
            }
            add(movelst, move);
        }
    }
}

/* DONE */
void generate_rook_moves(bitboard_t rooks, board_t *board, node_t *movelst){
    while(rooks){
        idx_t from = pop_1st_bit(&rooks);
        bitboard_t attacks = get_rook_attacks(from, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            move_t *move;
            if(is_capture(to, board)){
                move = generate_move(from, to, CAPTURE);
            }
            else{
                move = generate_move(from, to, QUIET);
            }
            add(movelst, move);
        }
    }
}

/* DONE */
void generate_queen_moves(bitboard_t queens, board_t *board, node_t *movelst){
    generate_bishop_moves(queens, board, movelst);
    generate_rook_moves(queens, board, movelst);
}

/* DONE */
void generate_white_pawn_moves(board_t *board, node_t *movelst){
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
                    add(movelst, generate_move(from, to, KCPROM));
                    add(movelst, generate_move(from, to, BCPROM));
                    add(movelst, generate_move(from, to, RCPROM));
                    add(movelst, generate_move(from, to, QCPROM));
                }
                else{
                    add(movelst, generate_move(from, to, KPROM));
                    add(movelst, generate_move(from, to, BPROM));
                    add(movelst, generate_move(from, to, RPROM));
                    add(movelst, generate_move(from, to, QPROM));
                }
            }
            /* if move is a double pawn push */ 
            else if((to - 16) == from){
                add(movelst, generate_move(from, to, DOUBLEP));
            }
            /* if move is a quiet or capture move */
            else{
                if(is_capture(to, board)){
                    add(movelst, generate_move(from, to, CAPTURE));
                }
                else{
                    add(movelst, generate_move(from, to, QUIET));
                }
            }
        }
    }
}

/* DONE */
void generate_black_pawn_moves(board_t *board, node_t *movelst){
    bitboard_t pawns = board->blackpawns;
    while(pawns){
        idx_t from = pop_1st_bit(&pawns);
        bitboard_t piece = (1ULL << from);
        bitboard_t attacks = get_white_pawn_attacks(piece, board);

        while(attacks){
            idx_t to = pop_1st_bit(&attacks);
            /* if move  is a promotion move */
            if(to >= 0 && to <= 7){
                if(is_capture(to, board)){
                    add(movelst, generate_move(from, to, KCPROM));
                    add(movelst, generate_move(from, to, BCPROM));
                    add(movelst, generate_move(from, to, RCPROM));
                    add(movelst, generate_move(from, to, QCPROM));
                }
                else{
                    add(movelst, generate_move(from, to, KPROM));
                    add(movelst, generate_move(from, to, BPROM));
                    add(movelst, generate_move(from, to, RPROM));
                    add(movelst, generate_move(from, to, QPROM));
                }
            }
            /* if move is a double pawn push */ 
            else if((to + 16) == from){
                add(movelst, generate_move(from, to, DOUBLEP));
            }
            /* if move is a quiet or capture move */
            else{
                if(is_capture(to, board)){
                    add(movelst, generate_move(from, to, CAPTURE));
                }
                else{
                    add(movelst, generate_move(from, to, QUIET));
                }
            }
        }
    }
}


void generate_white_enpassant_moves(board_t *board, node_t *movelst){
    bitboard_t pawns = board->whitepawns;
    if(board->ep_possible){
        bitboard_t ep = (1ULL << board->ep_field);
        while(pawns){
            idx_t from = pop_1st_bit(&pawns);
            bitboard_t piece = (1ULL << from);
            bitboard_t attacks = (((piece & CLEAR_FILE[A]) << 7) & ep) | (((piece & CLEAR_FILE[H]) << 9) & ep);

            while(attacks){
                idx_t to = pop_1st_bit(&attacks);
                add(movelst, generate_move(from, to, EPCAPTURE));
            }
        }
    }

    return;
} 

void generate_black_enpassant_moves(board_t *board, node_t *movelst){
    bitboard_t pawns = board->blackpawns;
    if(board->ep_possible){
        bitboard_t ep = (1ULL << board->ep_field);
        while(pawns){
            idx_t from = pop_1st_bit(&pawns);
            bitboard_t piece = (1ULL << from);
            bitboard_t attacks = (((piece & CLEAR_FILE[H]) >> 7) & ep) | (((piece & CLEAR_FILE[A]) >> 9) & ep);

            while(attacks){
                idx_t to = pop_1st_bit(&attacks);
                add(movelst, generate_move(from, to, EPCAPTURE));
            }
        }
    }
    
    return;
} 

void generate_white_castle_moves(board_t *board, node_t *movelst){
    // king/shortside
    if((board->all & ((1ULL << 5) | (1ULL << 6))) == 0 && (board->castle_rights & SHORTSIDEW)){
        add(movelst, generate_move(4, 6, KCASTLE));
    }
    // queen/longside
    if((board->all & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) == 0 && (board->castle_rights & LONGSIDEW)){
        add(movelst, generate_move(4, 2, QCASTLE));
    }
} 

void generate_black_castle_moves(board_t *board, node_t *movelst){
    // king/shortside
    if((board->all & ((1ULL << 61) | (1ULL << 62))) == 0 && (board->castle_rights & SHORTSIDEB)){
        add(movelst, generate_move(60, 62, KCASTLE));
    }
    // queen/longside
    if((board->all & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) == 0 && (board->castle_rights & LONGSIDEB)){
        add(movelst, generate_move(60, 58, QCASTLE));
    }
} 

void generate_pseudo_moves(board_t *board){
    node_t *movelst = init_list();

    if(board->player == WHITE){
        generate_knight_moves(board->whiteknights, board, movelst);
        generate_king_moves(board->whiteking, board, movelst);
        generate_bishop_moves(board->whitebishops, board, movelst);
        generate_rook_moves(board->whiterooks, board, movelst);
        generate_queen_moves(board->whitequeens, board, movelst);
        generate_white_pawn_moves(board, movelst);
        generate_white_enpassant_moves(board, movelst);
        generate_white_castle_moves(board, movelst);
    } else{
        generate_knight_moves(board->blackknights, board, movelst);
        generate_king_moves(board->blackking, board, movelst);
        generate_bishop_moves(board->blackbishops, board, movelst);
        generate_rook_moves(board->blackrooks, board, movelst);
        generate_queen_moves(board->blackqueens, board, movelst);
        generate_black_pawn_moves(board, movelst);
        generate_black_enpassant_moves(board, movelst);
        generate_black_castle_moves(board, movelst);
    }

    fprintf(stderr, "%d",len(movelst));
    while(len(movelst) != 0){
        move_t *move = pop(movelst);
        //print_move_on_board(move);
        free(move);
    }
    free(movelst);
}

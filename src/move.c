#include "../include/chess.h"

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS


/* Allocate memory for a move */
move_t *generate_move(idx_t from, idx_t to, flag_t flags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->from = from;
    move->to = to;
    move->flags = flags;
    return move;
}

/* Copy a move */
move_t *copy_move(move_t *move) {
    if (move == NULL) {
        return NULL;
    }

    move_t *copy = (move_t *) malloc(sizeof(move_t));
    
    copy->from = move->from;
    copy->to = move->to;
    copy->flags = move->flags;

    return copy;
}

/* Frees memory of move */
void free_move(move_t *move) {
    if (move) {
        free(move);
    }
}

/* Frees memory of move list */
void free_move_list(list_t *movelst) {
    move_t *move;
    while ((move = pop(movelst)) != NULL) {
        free_move(move);
    }

    if(movelst)free(movelst);
}

/* Execute move */
void do_move(board_t* board, move_t* move){
    /* save old board state */
    OLDSTATE[board->ply_no] = copy_board(board);

    /* execeute move */
    bitboard_t from_mask = 1ULL << move->from;
    bitboard_t from_clear_mask = ~from_mask;
    bitboard_t to_mask = 1ULL << move->to;
    bitboard_t to_clear_mask = ~to_mask;

    /* change bitboard of moving piece */
    /* PAWNS */
    if(from_mask & board->whitepawns){
        board->whitepawns |= to_mask;
        board->whitepawns &= from_clear_mask;
    }
    else if(from_mask & board->blackpawns){
        board->blackpawns |= to_mask;
        board->blackpawns &= from_clear_mask;
    }

    /* KNIGHTS */
    else if(from_mask & board->whiteknights){
        board->whiteknights |= to_mask;
        board->whiteknights &= from_clear_mask;
    }
    else if(from_mask & board->blackknights){
        board->blackknights |= to_mask;
        board->blackknights &= from_clear_mask;
    }

    /* BISHOPS */
    else if(from_mask & board->whitebishops){
        board->whitebishops |= to_mask;
        board->whitebishops &= from_clear_mask;
    }
    else if(from_mask & board->blackbishops){
        board->blackbishops |= to_mask;
        board->blackbishops &= from_clear_mask;
    }

    /* QUEENS */
    else if(from_mask & board->whitequeens){
        board->whitequeens |= to_mask;
        board->whitequeens &= from_clear_mask;
    }
    else if(from_mask & board->blackqueens){
        board->blackqueens |= to_mask;
        board->blackqueens &= from_clear_mask;
    }

    /* ROOKS */
    else if(from_mask & board->whiterooks){
        board->whiterooks |= to_mask;
        board->whiterooks &= from_clear_mask;
        if(move->from == 0) board->castle_rights &= ~(LONGSIDEW);
        if(move->from == 7) board->castle_rights &= ~(SHORTSIDEW);
    }
    else if(from_mask & board->blackrooks){
        board->blackrooks |= to_mask;
        board->blackrooks &= from_clear_mask;
        if(move->from == 56) board->castle_rights &= ~(LONGSIDEB);
        if(move->from == 63) board->castle_rights &= ~(SHORTSIDEB);
    }

    /* KINGS */
    else if(from_mask & board->whiteking){
        board->whiteking |= to_mask;
        board->whiteking &= from_clear_mask;
        board->castle_rights &= ~(SHORTSIDEW | LONGSIDEW);
    }
    else if(from_mask & board->blackking){
        board->blackking |= to_mask;
        board->blackking &= from_clear_mask;
        board->castle_rights &= ~(SHORTSIDEB | LONGSIDEB);
    }
    
    /* change bitboards of captured piece */ 
    if(move->flags & 0b0100){
        // adjust castle rights
        if(move->to == 7) board->castle_rights &= ~(SHORTSIDEW); 
        if(move->to == 0) board->castle_rights &= ~(LONGSIDEW); 
        if(move->to == 63) board->castle_rights &= ~(SHORTSIDEB); 
        if(move->to == 56) board->castle_rights &= ~(LONGSIDEB);

        // remove captured piece from board
        if(board->player == WHITE){
            board->black &= to_clear_mask;
            board->blackpawns &= board->black;
            board->blackknights &= board->black;
            board->blackbishops &= board->black;
            board->blackrooks &= board->black;
            board->blackqueens &= board->black;
            board->blackking &= board->black;
        } else {
            board->white &= to_clear_mask;
            board->whitepawns &= board->white;
            board->whiteknights &= board->white;
            board->whitebishops &= board->white;
            board->whiterooks &= board->white;
            board->whitequeens &= board->white;
            board->whiteking &= board->white;
        }
    }

    /////////////////////////////////////////////////
    // EXIT EARLY STATEMENTS BEGIN

    /* if quiet or simple capture move */
    if(move->flags == QUIET || move->flags == CAPTURE){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    /* if double pawn push */
    if(move->flags == DOUBLEP){
        board->ep_possible = TRUE;
        if(board->player == WHITE){
            board->ep_field = move->from+8;
        } else{
            board->ep_field = move->from-8;
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    /* if king side castle */
    if(move->flags == KCASTLE){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whiterooks |= (1ULL << 5);
            board->whiterooks &= ~(1ULL << 7);
            board->castle_rights &= ~(SHORTSIDEW | LONGSIDEW);
        } else{
            board->blackrooks |= (1ULL << 61);
            board->blackrooks &= ~(1ULL << 63);
            board->castle_rights &= ~(SHORTSIDEB | LONGSIDEB);
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    /* if queenside castle */
    if(move->flags == QCASTLE){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whiterooks |= (1ULL << 3);
            board->whiterooks &= ~(1ULL << 0);
            board->castle_rights &= ~(LONGSIDEW | SHORTSIDEW);
        } else{
            board->blackrooks |= (1ULL << 59);
            board->blackrooks &= ~(1ULL << 56);
            board->castle_rights &= ~(LONGSIDEB | SHORTSIDEB);
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }


    /* if promotion */
    if(move->flags == QPROM || move->flags == QCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whitequeens |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackqueens |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    if(move->flags == RPROM || move->flags == RCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whiterooks |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackrooks |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    if(move->flags == KPROM || move->flags == KCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whiteknights |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackknights |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    if(move->flags == BPROM || move->flags == BCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whitebishops |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackbishops |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }

    /* if ep capture */
    if(move->flags == EPCAPTURE){
        board->ep_possible = FALSE;
        board->ep_field = -1;
        
        if(board->player == WHITE){
            board->blackpawns &= ~(1ULL << (move->to-8));
        } else{
            board->whitepawns &= ~(1ULL << (move->to+8));
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        update_white_black_all_boards(board);
        return;
    }
    
}

/* Undo a move*/
void undo_move(board_t* board){
    board->ply_no--;
    board_t* old_board = OLDSTATE[board->ply_no];
    recover_board(board, old_board);
}
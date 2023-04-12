#include "../include/chess.h"

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

/* Determines if two mves are the same */
int is_same_move(move_t* move1, move_t* move2){
    // moves are considered same if their FROM & TO squares and FLAGS are the same
    // we do not consider the value of a move 
    if(move1->from != move2->from){
        return 0;
    }
    if(move1->to != move2->to){
        return 0;
    }
    if(move1->flags != move2->flags){
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
        // this should not happen but lets just handle it
        return NULL;
    }

    move_t *copy = (move_t *) malloc(sizeof(move_t));
    
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
void do_move(board_t* board, move_t* move){
    // FIRST: save old board state 
    OLDSTATE[board->ply_no] = copy_board(board);

    // move execution
    bitboard_t from_mask = 1ULL << move->from;
    bitboard_t from_clear_mask = ~from_mask;
    bitboard_t to_mask = 1ULL << move->to;
    bitboard_t to_clear_mask = ~to_mask;

    // change the bitboard of moving piece
    // if moving piece is a...

    // PAWN 
    if(from_mask & board->whitepawns){
        board->whitepawns |= to_mask;
        board->whitepawns &= from_clear_mask;
    }
    else if(from_mask & board->blackpawns){
        board->blackpawns |= to_mask;
        board->blackpawns &= from_clear_mask;
    }

    // KNIGHT
    else if(from_mask & board->whiteknights){
        board->whiteknights |= to_mask;
        board->whiteknights &= from_clear_mask;
    }
    else if(from_mask & board->blackknights){
        board->blackknights |= to_mask;
        board->blackknights &= from_clear_mask;
    }

    // BISHOP 
    else if(from_mask & board->whitebishops){
        board->whitebishops |= to_mask;
        board->whitebishops &= from_clear_mask;
    }
    else if(from_mask & board->blackbishops){
        board->blackbishops |= to_mask;
        board->blackbishops &= from_clear_mask;
    }

    // QUEEN
    else if(from_mask & board->whitequeens){
        board->whitequeens |= to_mask;
        board->whitequeens &= from_clear_mask;
    }
    else if(from_mask & board->blackqueens){
        board->blackqueens |= to_mask;
        board->blackqueens &= from_clear_mask;
    }

    // ROOK 
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

    // KING 
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
    
    // now we change the bitboard of captured piece
    if(move->flags & 0b0100){
        // first off, adjust castle rights if rooks were captured 
        if(move->to == 7) board->castle_rights &= ~(SHORTSIDEW); 
        if(move->to == 0) board->castle_rights &= ~(LONGSIDEW); 
        if(move->to == 63) board->castle_rights &= ~(SHORTSIDEB); 
        if(move->to == 56) board->castle_rights &= ~(LONGSIDEB);

        // then, remove captured piece from (bit)board
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

    //
    //  EXIT EARLY STATEMENTS BEGIN
    //

    // if move is quiet or simple capture
    if(move->flags == QUIET || move->flags == CAPTURE){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is double pawn push
    if(move->flags == DOUBLEP){
        // enpassant capture is possible next round
        board->ep_possible = TRUE;
        if(board->player == WHITE){
            board->ep_field = move->from+8;
        } else{
            board->ep_field = move->from-8;
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is kingside castle 
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
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is queenside castle 
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
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }


    // if move is a queen promotion move 
    if(move->flags == QPROM || move->flags == QCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;
        
        // remove pawn from to square and replace it by a queen
        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whitequeens |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackqueens |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is a rook promotion move 
    if(move->flags == RPROM || move->flags == RCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        // remove pawn from to square and replace it by a rook
        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whiterooks |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackrooks |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is a knight promotion move 
    if(move->flags == KPROM || move->flags == KCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        // remove pawn from to square and replace it by a knight
        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whiteknights |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackknights |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is a bishop promotion move 
    if(move->flags == BPROM || move->flags == BCPROM){
        board->ep_possible = FALSE;
        board->ep_field = -1;

        // remove pawn from to square and replace it by a bishop
        if(board->player == WHITE){
            board->whitepawns &= to_clear_mask;
            board->whitebishops |= to_mask;
        }else {
            board->blackpawns &= to_clear_mask;
            board->blackbishops |= to_mask;
        }
        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }

    // if move is an ep capture
    if(move->flags == EPCAPTURE){
        board->ep_possible = FALSE;
        board->ep_field = -1;
        
        // special handling of removance of captured piece
        if(board->player == WHITE){
            board->blackpawns &= ~(1ULL << (move->to-8));
        } else{
            board->whitepawns &= ~(1ULL << (move->to+8));
        }

        board->ply_no++;
        board->player = (board->player == WHITE)?(BLACK):(WHITE); 
        // dont forget to update the white, black and all bitboard
        update_white_black_all_boards(board);
        return;
    }
    
}

/* Undos a move*/
void undo_move(board_t* board){
    // reduce ply number
    board->ply_no--;
    // and recover old board state from board state saved for that exact ply
    board_t* old_board = OLDSTATE[board->ply_no];
    recover_board(board, old_board);
}
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
    OLDSTATE[board->ply_no] = copy_board(board);

    if(move->flags == QUIET || move->flags == CAPTURE){
        if(board->player == WHITE){
            if(board->whitepawns & (1ULL << move->from)){
                board->whitepawns |= (1ULL << move->to);
                board->whitepawns &= ~(1ULL << move->from);
            }
            else if(board->whiteknights & (1ULL << move->from)){
                board->whiteknights |= (1ULL << move->to);
                board->whiteknights &= ~(1ULL << move->from);
            }
            else if(board->whitebishops & (1ULL << move->from)){
                board->whitebishops |= (1ULL << move->to);
                board->whitebishops &= ~(1ULL << move->from);
            }
            else if(board->whiterooks & (1ULL << move->from)){
                board->whiterooks |= (1ULL << move->to);
                board->whiterooks &= ~(1ULL << move->from);
            }
            else if(board->whitequeens & (1ULL << move->from)){
                board->whitequeens |= (1ULL << move->to);
                board->whitequeens &= ~(1ULL << move->from);
            }
            else if(board->whiteking & (1ULL << move->from)){
                board->whiteking |= (1ULL << move->to);
                board->whiteking &= ~(1ULL << move->from);
            }
            else{
                fprintf(stderr, "Couldnt find piece on from square (white)...ERROR\n");
                print_board(board);
                fprintf(stderr, "From:%d To:%d Flags:%d\n", move->from, move->to, move->flags);
                exit(1);
            }

            if(move->flags == CAPTURE){
                board->black &= ~(1ULL << move->to);
                board->blackpawns &= board->black;
                board->blackknights &= board->black;
                board->blackbishops &= board->black;
                board->blackrooks &= board->black;
                board->blackqueens &= board->black;
                board->blackking &= board->black;
            }
            
        } else {
            if(board->blackpawns & (1ULL << move->from)){
                board->blackpawns |= (1ULL << move->to);
                board->blackpawns &= ~(1ULL << move->from);
            }
            else if(board->blackknights & (1ULL << move->from)){
                board->blackknights |= (1ULL << move->to);
                board->blackknights &= ~(1ULL << move->from);
            }
            else if(board->blackbishops & (1ULL << move->from)){
                board->blackbishops |= (1ULL << move->to);
                board->blackbishops &= ~(1ULL << move->from);
            }
            else if(board->blackrooks & (1ULL << move->from)){
                board->blackrooks |= (1ULL << move->to);
                board->blackrooks &= ~(1ULL << move->from);
            }
            else if(board->blackqueens & (1ULL << move->from)){
                board->blackqueens |= (1ULL << move->to);
                board->blackqueens &= ~(1ULL << move->from);
            }
            else if(board->blackking & (1ULL << move->from)){
                board->blackking |= (1ULL << move->to);
                board->blackking &= ~(1ULL << move->from);
            }
            else{

                fprintf(stderr, "Couldnt find piece on from square (black)...ERROR\n");
                print_board(board);
                fprintf(stderr, "From:%d To:%d Flags:%x\n", move->from, move->to, move->flags);
                exit(1);
            }

            if(move->flags == CAPTURE){
                board->white &= ~(1ULL << move->to);
                board->whitepawns &= board->white;
                board->whiteknights &= board->white;
                board->whitebishops &= board->white;
                board->whiterooks &= board->white;
                board->whitequeens &= board->white;
                board->whiteking &= board->white;
            }
        }
    }
    else if(move->flags == DOUBLEP){
        board->ep_possible = TRUE;
        if(board->player == WHITE){
            board->whitepawns |= (1ULL << move->to);
            board->whitepawns &= ~(1ULL << move->from);
            board->ep_field = move->from+8;
        } else{
            board->blackpawns |= (1ULL << move->to);
            board->blackpawns &= ~(1ULL << move->from);
            board->ep_field = move->from-8;
        }
    }
    else if(move->flags == KCASTLE){
        if(board->player == WHITE){
            board->whiteking = (1ULL << 6);
            board->whiterooks |= (1ULL << 5);
            board->whiterooks &= ~(1ULL << 7);
            board->castle_rights &= ~(SHORTSIDEW);
        } else{
            board->blackking = (1ULL << 62);
            board->blackrooks |= (1ULL << 61);
            board->blackrooks &= ~(1ULL << 63);
            board->castle_rights &= ~(SHORTSIDEB);
        }
    }
    else if(move->flags == QCASTLE){
        if(board->player == WHITE){
            board->whiteking = (1ULL << 2);
            board->whiterooks |= (1ULL << 3);
            board->whiterooks &= ~(1ULL << 0);
            board->castle_rights &= ~(LONGSIDEW);
        } else{
            board->blackking = (1ULL << 58);
            board->blackrooks |= (1ULL << 59);
            board->blackrooks &= ~(1ULL << 56);
            board->castle_rights &= ~(LONGSIDEB);
        }
    }
    else if(move->flags == EPCAPTURE){
        if(board->player == WHITE){
            board->whitepawns |= (1ULL << move->to);
            board->whitepawns &= ~(1ULL << move->from);
            board->blackpawns &= ~(1ULL << (move->to-8));
        } else{
            board->blackpawns |= (1ULL << move->to);
            board->blackpawns &= ~(1ULL << move->from);
            board->whitepawns &= ~(1ULL << (move->to+8));
        }
    }
    else if(move->flags == KPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whiteknights |= (1ULL << move->to);
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackknights |= (1ULL << move->to);
        }
    }
    else if(move->flags == BPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whitebishops |= (1ULL << move->to);
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackbishops |= (1ULL << move->to);
        }
    }
    else if(move->flags == RPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whiterooks |= (1ULL << move->to);
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackrooks |= (1ULL << move->to);
        }
    }
    else if(move->flags == QPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whitequeens |= (1ULL << move->to);
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackqueens |= (1ULL << move->to);
        }
    }
    else if(move->flags == KCPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whiteknights |= (1ULL << move->to);

            board->black &= ~(1ULL << move->to);
            board->blackpawns &= board->black;
            board->blackknights &= board->black;
            board->blackbishops &= board->black;
            board->blackrooks &= board->black;
            board->blackqueens &= board->black;
            board->blackking &= board->black;
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackknights |= (1ULL << move->to);

            board->white &= ~(1ULL << move->to);
            board->whitepawns &= board->white;
            board->whiteknights &= board->white;
            board->whitebishops &= board->white;
            board->whiterooks &= board->white;
            board->whitequeens &= board->white;
            board->whiteking &= board->white;
        }
    }
    else if(move->flags == BCPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whitebishops |= (1ULL << move->to);

            board->black &= ~(1ULL << move->to);
            board->blackpawns &= board->black;
            board->blackknights &= board->black;
            board->blackbishops &= board->black;
            board->blackrooks &= board->black;
            board->blackqueens &= board->black;
            board->blackking &= board->black;
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackbishops |= (1ULL << move->to);

            board->white &= ~(1ULL << move->to);
            board->whitepawns &= board->white;
            board->whiteknights &= board->white;
            board->whitebishops &= board->white;
            board->whiterooks &= board->white;
            board->whitequeens &= board->white;
            board->whiteking &= board->white;
        }
    }
    else if(move->flags == RCPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whiterooks |= (1ULL << move->to);

            board->black &= ~(1ULL << move->to);
            board->blackpawns &= board->black;
            board->blackknights &= board->black;
            board->blackbishops &= board->black;
            board->blackrooks &= board->black;
            board->blackqueens &= board->black;
            board->blackking &= board->black;
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackrooks |= (1ULL << move->to);

            board->white &= ~(1ULL << move->to);
            board->whitepawns &= board->white;
            board->whiteknights &= board->white;
            board->whitebishops &= board->white;
            board->whiterooks &= board->white;
            board->whitequeens &= board->white;
            board->whiteking &= board->white;
        }
    }
    else if(move->flags == QCPROM){
        if(board->player == WHITE){
            board->whitepawns &= ~(1ULL << move->from);
            board->whitequeens |= (1ULL << move->to);

            board->black &= ~(1ULL << move->to);
            board->blackpawns &= board->black;
            board->blackknights &= board->black;
            board->blackbishops &= board->black;
            board->blackrooks &= board->black;
            board->blackqueens &= board->black;
            board->blackking &= board->black;
        } else {
            board->blackpawns &= ~(1ULL << move->from);
            board->blackqueens |= (1ULL << move->to);

            board->white &= ~(1ULL << move->to);
            board->whitepawns &= board->white;
            board->whiteknights &= board->white;
            board->whitebishops &= board->white;
            board->whiterooks &= board->white;
            board->whitequeens &= board->white;
            board->whiteking &= board->white;
        }
    }
    else{
        fprintf(stderr, "Encountered a invalid move flag %d...ERROR\n", move->flags);
        exit(1);
    }

    if(move->flags != DOUBLEP){
        board->ep_possible = FALSE;
        board->ep_field = -1;
    }

    board->ply_no++;
    board->player = (board->player == WHITE)?(BLACK):(WHITE); 

    update_white_black_all_boards(board);
}

/* Undo a move*/
void undo_move(board_t* board, move_t* move){
    board->ply_no--;
    board_t* old_board = OLDSTATE[board->ply_no];
    recover_board(board, old_board);
}
#include "../include/chess.h"

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

move_t *copy_move(move_t *move) {
    if (move == NULL) {
        return NULL;
    }
    move_t *copy = (move_t *)malloc(sizeof(move_t));

    copy->typeofmove = move->typeofmove;
    copy->start = move->start;
    copy->end = move->end;
    copy->startopt = move->startopt;
    copy->endopt = move->endopt;
    copy->piece_was = move->piece_was;
    copy->piece_is = move->piece_is;
    copy->piece_cap = move->piece_cap;

    copy->newcr = move->newcr;
    copy->newep = move->newep;
    copy->orderid = move->orderid;

    copy->oldflags = copyflagsfrommove(move);

    return copy;
}

/* Allocate memory for a move */
move_t *create_normalmove(piece_t piece_was, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->typeofmove = NORMALMOVE;
    move->orderid = (FIGURE(piece_cap) * 100) + (KING - FIGURE(piece_was));

    move->piece_was = piece_was;
    move->piece_cap = piece_cap;

    move->start = start;
    move->end = end;

    move->newcr = newcr;

    move->oldflags = oldflags;
    return move;
}

move_t *create_eppossiblemove(piece_t piece_was, idx_t start, idx_t end, flag_t newepfield, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->typeofmove = EPPOSSIBLEMOVE;
    move->orderid = 0;

    move->piece_was = piece_was;

    move->start = start;
    move->end = end;

    move->newep = newepfield;

    move->oldflags = oldflags;
    return move;
}

move_t *create_promotionmove(piece_t piece_was, piece_t piece_is, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->typeofmove = PROMOTIONMOVE;
    move->orderid = 10000 + (FIGURE(piece_is) * 100) + (FIGURE(piece_cap));

    move->piece_was = piece_was;
    move->piece_is = piece_is;
    move->piece_cap = piece_cap;

    move->start = start;
    move->end = end;

    move->newcr = newcr;

    move->oldflags = oldflags;
    return move;
}

move_t *create_castlemove(idx_t startking, idx_t endking, idx_t startrook, idx_t endrook, flag_t newcr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));
    move->typeofmove = CASTLEMOVE;
    move->orderid = 0;

    move->start = startking;
    move->end = endking;
    move->startopt = startrook;
    move->endopt = endrook;

    move->newcr = newcr;

    move->oldflags = oldflags;

    return move;
}

move_t *create_epmove(idx_t startattacker, idx_t endattacker, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));
    move->typeofmove = ENPASSANTMOVE;
    move->orderid = 0;

    move->start = startattacker;
    move->end = endattacker;

    move->oldflags = oldflags;
    return (move);
}

/* Frees memory of move */
void free_move(move_t *move) {
    if (move) {
        free(move->oldflags);
        free(move);
    }
}

void free_movelst(node_t *movelst) {
    move_t *move;
    while ((move = pop(movelst)) != NULL) {
        free_move(move);
    }

    free(movelst);
}

/* Execute move */
void playMove(board_t *board, move_t *move, player_t playerwhomademove) {
    if (move->typeofmove == NORMALMOVE) {
        // start and end field
        board->playingfield[move->start] = EMPTY;
        board->playingfield[move->end] = move->piece_was;

        // castle rights
        board->castlerights = move->newcr;

        // en passant
        board->eppossible = FALSE;
        board->epfield = 0;
    }

    else if (move->typeofmove == PROMOTIONMOVE) {
        // start and end field
        board->playingfield[move->start] = EMPTY;
        board->playingfield[move->end] = move->piece_is;

        // castle rights
        board->castlerights = move->newcr;

        // en passant
        board->eppossible = FALSE;
        board->epfield = 0;
    }

    else if (move->typeofmove == EPPOSSIBLEMOVE) {
        // start and end field
        board->playingfield[move->start] = EMPTY;
        board->playingfield[move->end] = move->piece_was;

        // castle rights
        board->castlerights = board->castlerights;

        // en passant
        board->eppossible = TRUE;
        board->epfield = move->newep;
    }

    else if (move->typeofmove == CASTLEMOVE) {
        // start and end field
        board->playingfield[move->start] = EMPTY;
        board->playingfield[move->end] = (move->end <= 7) ? KING : (KING | BLACK);
        board->playingfield[move->startopt] = EMPTY;
        board->playingfield[move->endopt] = (move->endopt <= 7) ? ROOK : (ROOK | BLACK);

        // castle rights
        board->castlerights = move->newcr;

        // en passant
        board->eppossible = FALSE;
        board->epfield = 0;
    }

    else if (move->typeofmove == ENPASSANTMOVE) {
        // start and end field
        board->playingfield[move->start] = EMPTY;
        board->playingfield[move->end] = (playerwhomademove == WHITE) ? PAWN : (PAWN | BLACK);
        idx_t captured = (playerwhomademove == WHITE) ? (move->end - 8) : (move->end + 8);
        board->playingfield[captured] = EMPTY;

        // castle rights
        board->castlerights = board->castlerights;

        // en passant
        board->eppossible = FALSE;
        board->epfield = 0;
    }

    // switch player
    board->player = OPPONENT(playerwhomademove);
}

/* Reverses a move/ recovers old board state */
void reverseMove(board_t *board, move_t *move, player_t playerwhomademove) {
    if (move->typeofmove == NORMALMOVE) {
        // start and end field
        board->playingfield[move->start] = move->piece_was;
        board->playingfield[move->end] = move->piece_cap;
    }

    else if (move->typeofmove == PROMOTIONMOVE) {
        // start and end field
        board->playingfield[move->start] = move->piece_was;
        board->playingfield[move->end] = move->piece_cap;
    }

    else if (move->typeofmove == EPPOSSIBLEMOVE) {
        // start and end field
        board->playingfield[move->start] = move->piece_was;
        board->playingfield[move->end] = EMPTY;
    }

    else if (move->typeofmove == CASTLEMOVE) {
        // start and end field
        board->playingfield[move->start] = (move->end <= 7) ? KING : (KING | BLACK);
        board->playingfield[move->end] = EMPTY;
        board->playingfield[move->startopt] = (move->endopt <= 7) ? ROOK : (ROOK | BLACK);
        board->playingfield[move->endopt] = EMPTY;
    }

    else if (move->typeofmove == ENPASSANTMOVE) {
        // start and end field
        board->playingfield[move->start] = (playerwhomademove == WHITE) ? PAWN : (PAWN | BLACK);
        board->playingfield[move->end] = EMPTY;
        idx_t captured = (playerwhomademove == WHITE) ? (move->end - 8) : (move->end + 8);
        board->playingfield[captured] = (playerwhomademove == WHITE) ? (PAWN | BLACK) : PAWN;
    }

    // castle rights
    board->castlerights = move->oldflags->castlerights;

    // en passant
    board->eppossible = move->oldflags->eppossible;
    board->epfield = move->oldflags->epfield;

    // switch player
    board->player = playerwhomademove;
}

int isSameMove(move_t *move, move_t *move2) {
    if (move->typeofmove != move2->typeofmove) {
        return 0;
    }
    if (move->start != move2->start) {
        return 0;
    }
    if (move->end != move2->end) {
        return 0;
    }
    if (move->piece_was != move2->piece_was) {
        return 0;
    }
    if (move->piece_cap != move2->piece_cap) {
        return 0;
    }
    if (move->typeofmove == PROMOTIONMOVE) {
        if (move->piece_is != move2->piece_is) {
            return 0;
        }
    }

    return 1;
}

int PVMoveIsPossible(node_t *movelst, move_t *ttmove) {
    node_t *tmp = movelst;
    if (ttmove != NULL) {
        while (tmp->next != NULL) {
            if (isSameMove(tmp->next->move, ttmove) == 1) {
                return 1;
                break;
            }
            tmp = tmp->next;
        }
    }
    return 0;
}

node_t *sortMoves(node_t *head) {
    node_t *sorted = init_list();

    // if list is empty
    if (head->next == NULL) {
        return head;
    }

    while (len(head) > 0) {
        node_t *tmp = head->next;

        // initially first move in list is worst move (and head the previous node)
        node_t *prev = head;
        node_t *worstmove = tmp;

        // find worst move by orderid
        while (tmp->next != NULL) {
            if (tmp->next->move->orderid < worstmove->move->orderid) {
                // save the worst move and the previous node
                prev = tmp;
                worstmove = tmp->next;
            }
            tmp = tmp->next;
        }

        // remove worst move from list by adjusting pointer from previous node
        prev->next = worstmove->next;

        // add move to sorted list (from worst to best)
        add(sorted, copy_move(worstmove->move));

        // free node of worstmove
        free_move(worstmove->move);
        free(worstmove);
    }
    free(head);

    return sorted;
}
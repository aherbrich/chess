#include "../include/chess.h"

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

move_t *copy_move(move_t *move) {
    if (move == NULL) {
        return NULL;
    }
    move_t *copy = (move_t *)malloc(sizeof(move_t));

    copy->type_of_move = move->type_of_move;
    copy->start = move->start;
    copy->end = move->end;
    copy->optional_start = move->optional_start;
    copy->optional_end = move->optional_end;
    copy->piece_was = move->piece_was;
    copy->piece_is = move->piece_is;
    copy->piece_cap = move->piece_cap;

    copy->new_cr = move->new_cr;
    copy->new_ep = move->new_ep;
    copy->order_id = move->order_id;

    copy->oldflags = copy_flags_from_move(move);

    return copy;
}

/* Allocate memory for a move */
move_t *create_normal_move(piece_t piece_was, piece_t piece_cap, idx_t start, idx_t end, flag_t new_cr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->type_of_move = NORMALMOVE;
    move->order_id = (FIGURE(piece_cap) * 100) + (KING - FIGURE(piece_was));

    move->piece_was = piece_was;
    move->piece_cap = piece_cap;

    move->start = start;
    move->end = end;

    move->new_cr = new_cr;

    move->oldflags = oldflags;
    return move;
}

move_t *create_ep_possible_move(piece_t piece_was, idx_t start, idx_t end, flag_t new_ep_field, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->type_of_move = EPPOSSIBLEMOVE;
    move->order_id = 0;

    move->piece_was = piece_was;

    move->start = start;
    move->end = end;

    move->new_ep = new_ep_field;

    move->oldflags = oldflags;
    return move;
}

move_t *create_promotion_move(piece_t piece_was, piece_t piece_is, piece_t piece_cap, idx_t start, idx_t end, flag_t new_cr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->type_of_move = PROMOTIONMOVE;
    move->order_id = 10000 + (FIGURE(piece_is) * 100) + (FIGURE(piece_cap));

    move->piece_was = piece_was;
    move->piece_is = piece_is;
    move->piece_cap = piece_cap;

    move->start = start;
    move->end = end;

    move->new_cr = new_cr;

    move->oldflags = oldflags;
    return move;
}

move_t *create_castle_move(idx_t start_king, idx_t end_king, idx_t start_rook, idx_t end_rook, flag_t new_cr, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));
    move->type_of_move = CASTLEMOVE;
    move->order_id = 0;

    move->start = start_king;
    move->end = end_king;
    move->optional_start = start_rook;
    move->optional_end = end_rook;

    move->new_cr = new_cr;

    move->oldflags = oldflags;

    return move;
}

move_t *create_ep_move(idx_t start_attacker, idx_t end_attacker, oldflags_t *oldflags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));
    move->type_of_move = ENPASSANTMOVE;
    move->order_id = 0;

    move->start = start_attacker;
    move->end = end_attacker;

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

/* Frees memory of move list */
void free_move_list(node_t *move_list) {
    move_t *move;
    while ((move = pop(move_list)) != NULL) {
        free_move(move);
    }

    free(move_list);
}

/* Execute move */
void play_move(board_t *board, move_t *move, player_t player_who_made_move) {
    if (move->type_of_move == NORMALMOVE) {
        // start and end field
        board->playing_field[move->start] = EMPTY;
        board->playing_field[move->end] = move->piece_was;

        // castle rights
        board->castle_rights = move->new_cr;

        // en passant
        board->ep_possible = FALSE;
        board->ep_field = 0;
    }

    else if (move->type_of_move == PROMOTIONMOVE) {
        // start and end field
        board->playing_field[move->start] = EMPTY;
        board->playing_field[move->end] = move->piece_is;

        // castle rights
        board->castle_rights = move->new_cr;

        // en passant
        board->ep_possible = FALSE;
        board->ep_field = 0;
    }

    else if (move->type_of_move == EPPOSSIBLEMOVE) {
        // start and end field
        board->playing_field[move->start] = EMPTY;
        board->playing_field[move->end] = move->piece_was;

        // castle rights
        board->castle_rights = board->castle_rights;

        // en passant
        board->ep_possible = TRUE;
        board->ep_field = move->new_ep;
    }

    else if (move->type_of_move == CASTLEMOVE) {
        // start and end field
        board->playing_field[move->start] = EMPTY;
        board->playing_field[move->end] = (move->end <= 7) ? KING : (KING | BLACK);
        board->playing_field[move->optional_start] = EMPTY;
        board->playing_field[move->optional_end] = (move->optional_end <= 7) ? ROOK : (ROOK | BLACK);

        // castle rights
        board->castle_rights = move->new_cr;

        // en passant
        board->ep_possible = FALSE;
        board->ep_field = 0;
    }

    else if (move->type_of_move == ENPASSANTMOVE) {
        // start and end field
        board->playing_field[move->start] = EMPTY;
        board->playing_field[move->end] = (player_who_made_move == WHITE) ? PAWN : (PAWN | BLACK);
        idx_t captured = (player_who_made_move == WHITE) ? (move->end - 8) : (move->end + 8);
        board->playing_field[captured] = EMPTY;

        // castle rights
        board->castle_rights = board->castle_rights;

        // en passant
        board->ep_possible = FALSE;
        board->ep_field = 0;
    }

    // switch player
    board->player = OPPONENT(player_who_made_move);
}

/* Reverses a move/ recovers old board state */
void reverse_move(board_t *board, move_t *move, player_t player_who_made_move) {
    if (move->type_of_move == NORMALMOVE) {
        // start and end field
        board->playing_field[move->start] = move->piece_was;
        board->playing_field[move->end] = move->piece_cap;
    }

    else if (move->type_of_move == PROMOTIONMOVE) {
        // start and end field
        board->playing_field[move->start] = move->piece_was;
        board->playing_field[move->end] = move->piece_cap;
    }

    else if (move->type_of_move == EPPOSSIBLEMOVE) {
        // start and end field
        board->playing_field[move->start] = move->piece_was;
        board->playing_field[move->end] = EMPTY;
    }

    else if (move->type_of_move == CASTLEMOVE) {
        // start and end field
        board->playing_field[move->start] = (move->end <= 7) ? KING : (KING | BLACK);
        board->playing_field[move->end] = EMPTY;
        board->playing_field[move->optional_start] = (move->optional_end <= 7) ? ROOK : (ROOK | BLACK);
        board->playing_field[move->optional_end] = EMPTY;
    }

    else if (move->type_of_move == ENPASSANTMOVE) {
        // start and end field
        board->playing_field[move->start] = (player_who_made_move == WHITE) ? PAWN : (PAWN | BLACK);
        board->playing_field[move->end] = EMPTY;
        idx_t captured = (player_who_made_move == WHITE) ? (move->end - 8) : (move->end + 8);
        board->playing_field[captured] = (player_who_made_move == WHITE) ? (PAWN | BLACK) : PAWN;
    }

    // castle rights
    board->castle_rights = move->oldflags->castle_rights;

    // en passant
    board->ep_possible = move->oldflags->ep_possible;
    board->ep_field = move->oldflags->ep_field;

    // switch player
    board->player = player_who_made_move;
}

/* Returns true if two moves are identical */
int is_same_move(move_t *move, move_t *move2) {
    if (move->type_of_move != move2->type_of_move) {
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
    if (move->type_of_move == PROMOTIONMOVE) {
        if (move->piece_is != move2->piece_is) {
            return 0;
        }
    }

    return 1;
}

int PVMove_is_possible(node_t *move_list, move_t *tt_move) {
    node_t *tmp = move_list;
    if (tt_move != NULL) {
        while (tmp->next != NULL) {
            if (is_same_move(tmp->next->move, tt_move) == 1) {
                return 1;
                break;
            }
            tmp = tmp->next;
        }
    }
    return 0;
}

/* Sorts list by capture order */
node_t *sort_moves(node_t *head) {
    node_t *sorted = init_list();

    // if list is empty
    if (head->next == NULL) {
        return head;
    }

    while (len(head) > 0) {
        node_t *tmp = head->next;

        // initially first move in list is worst move (and head the previous node)
        node_t *prev = head;
        node_t *worst_move = tmp;

        // find worst move by order_id
        while (tmp->next != NULL) {
            if (tmp->next->move->order_id < worst_move->move->order_id) {
                // save the worst move and the previous node
                prev = tmp;
                worst_move = tmp->next;
            }
            tmp = tmp->next;
        }

        // remove worst move from list by adjusting pointer from previous node
        prev->next = worst_move->next;

        // add move to sorted list (from worst to best)
        add(sorted, copy_move(worst_move->move));

        // free node of worst_move
        free_move(worst_move->move);
        free(worst_move);
    }
    free(head);

    return sorted;
}

/* Converts a string to a move index */
idx_t str_to_idx(char *ptr, int len) {
    idx_t idx = 0;
    for(int i = 0; i < len; i++) {
        switch (*ptr) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
                idx = idx + ((*ptr) - 'a');
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
                idx = idx + ((*ptr) - 'A');
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                idx = idx + (8 * (*ptr - '1'));
                break;
        }
        ptr++;
    }
    return (idx);
}

/* Converts a character to a piece */
piece_t str_to_piece(char p_char) {
    switch (p_char) {
        case 'p':
            return(PAWN | BLACK);
            break;
        case 'n':
            return(KNIGHT | BLACK);
            break;
        case 'b':
            return(BISHOP | BLACK);
            break;
        case 'r':
            return(ROOK | BLACK);
            break;
        case 'q':
            return(QUEEN | BLACK);
            break;
        case 'k':
            return(KING | BLACK);
            break;
        case 'P':
            return(PAWN | WHITE);
            break;
        case 'N':
            return(KNIGHT | WHITE);
            break;
        case 'B':
            return(BISHOP | WHITE);
            break;
        case 'R':
            return(ROOK | WHITE);
            break;
        case 'Q':
            return(QUEEN | WHITE);
            break;
        case 'K':
            return(KING | WHITE);
            break;
        default:
            fprintf(stderr, "parsing error ('%c')\n", p_char);
            exit(-1);
    }

    fprintf(stderr, "parsing error ('%c')\n", p_char);
    exit(-1);
    return(-1);
}

/* Converts a string to a move in the context of an existing board */
move_t *str_to_move(board_t *board, char *move_str) {
    /* extracts the from and to position of the move */
    idx_t from = str_to_idx(move_str, 2);
    idx_t to = str_to_idx(&(move_str[2]), 2);
    piece_t new_piece = (strlen(move_str) == 5) ? str_to_piece(move_str[4]) : EMPTY;

    /* generate all possible moves in the current position ... */
    node_t *move_list = generate_moves(board);
    node_t *node = move_list;
    move_t *move = NULL;

    while ((node = node->next)) {
        if (node->move->start == from && node->move->end == to) {
            if ((node->move->type_of_move != PROMOTIONMOVE && new_piece == EMPTY) ||
                (node->move->type_of_move == PROMOTIONMOVE && node->move->piece_is == new_piece)) {
                move = copy_move(node->move);
                break;
            }
        }
    }
    free_move_list(move_list);

    return move;
}
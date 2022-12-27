#include "../include/chess.h"
#include "../include/zobrist.h"

char FIELD[64][2] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
                     "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                     "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                     "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                     "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                     "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                     "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                     "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

/* Print a piece */
char print_piece(piece_t piece) {
    switch (piece) {
        case 2:
            return 'P';
        case 4:
            return 'N';
        case 8:
            return 'B';
        case 16:
            return 'R';
        case 32:
            return 'Q';
        case 64:
            return 'K';
        case 3:
            return 'p';
        case 5:
            return 'n';
        case 9:
            return 'b';
        case 17:
            return 'r';
        case 33:
            return 'q';
        case 65:
            return 'k';
        default:
            return '-';
    }
}

/* Print the board */
void print_board(board_t* board) {
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }

            char piece = print_piece(board->playing_field[pos_to_idx(x, y)]);
            if (COLOR(board->playing_field[pos_to_idx(x, y)]) == BLACK) {
                fprintf(stderr, "%s", Color_CYAN);
            }
            fprintf(stderr, "%c%s  ", piece, Color_END);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN, Color_END);
    fprintf(stderr, "\n");
}

/* Print move */
void print_move(move_t* move) {
    char* start_field = FIELD[move->start];
    char* end_field = FIELD[move->end];
    if (move->type_of_move == PROMOTIONMOVE) {
        fprintf(stderr, "%s%.2s-%.2s-(%c)%s",Color_PURPLE, start_field, end_field, print_piece(move->piece_is),Color_END);

    } else if (move->type_of_move == CASTLEMOVE) {
        fprintf(stderr, "%s%.2s-%.2s-C%s", Color_PURPLE, start_field, end_field, Color_END);
    } else {
        fprintf(stderr, "%s%.2s-%.2s-%c%s",Color_PURPLE, start_field, end_field, print_piece(move->piece_was), Color_END);
    }
}

/* Print move in the long algebraic notation */
void print_LAN_move(move_t* move) {
    char* start_field = FIELD[move->start];
    char* end_field = FIELD[move->end];

    if (move->type_of_move == PROMOTIONMOVE)
        printf("%.2s%.2s%c", start_field, end_field, print_piece(move->piece_is));
    else 
        printf("%.2s%.2s", start_field, end_field);

    return;
}

/* Print list move */
void print_moves(node_t* move_list) {
    node_t* ptr = move_list->next;
    while (ptr != NULL) {
        print_move(ptr->move);
        fprintf(stderr, "\n");
        ptr = ptr->next;
    }
    fprintf(stderr, "\n");
}

/* Print the list of best possible moves until a depth (PV line) */
void print_line(board_t* board, int depth) {
    /* make a copy of the board */
    board_t *board_copy = copy_board(board);

    /* for all depth, figure out the best move from the hash-table and print it */
    for (int d = depth; d > 0; d--) {
        move_t* best_move = get_best_move_from_hashtable(board_copy);
        if (best_move == NULL) {
            free_board(board_copy);
            return;
        }

        play_move(board_copy, best_move, board_copy->player);
        print_LAN_move(best_move);
        printf(" ");
        free_move(best_move);
    }

    /* free the board */
    free_board(board_copy);

    return;
}
#include "include/engine-core/prettyprint.h"

#include <stdio.h>

#include "include/engine-core/prettyprint.h"

#include "include/engine-core/types.h"
#include "include/engine-core/board.h"
#include "include/engine-core/helpers.h"
#include "include/engine-core/move.h"
#include "include/engine-core/zobrist.h"
#include "include/engine-core/tt.h"

char FIELD[64][2] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2",
                     "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3",
                     "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4",
                     "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                     "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7",
                     "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8",
                     "e8", "f8", "g8", "h8"};

char PIECE[NR_PIECES] = {'p', 'n', 'b', 'r', 'q', 'k', ' ', ' ', 'P', 'N', 'B', 'R', 'Q', 'K', 'E'};
char PROM[9] = {'n', 'b', 'r', 'q', 'N', 'B', 'R', 'Q', 'E'};

/* Creates a string representing the board */
char* create_board_string(board_t* board) {
    char* string = (char*)malloc(64);
    for (int i = 0; i < 64; i++) {
        string[i] = '-';
    }

    int idx;
    bitboard_t copy = board->piece_bb[W_PAWN];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'P';
    }
    copy = board->piece_bb[B_PAWN];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'p';
    }
    copy = board->piece_bb[W_KNIGHT];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'N';
    }
    copy = board->piece_bb[B_KNIGHT];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'n';
    }
    copy = board->piece_bb[W_BISHOP];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'B';
    }
    copy = board->piece_bb[B_BISHOP];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'b';
    }
    copy = board->piece_bb[W_ROOK];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'R';
    }
    copy = board->piece_bb[B_ROOK];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'r';
    }
    copy = board->piece_bb[W_QUEEN];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'Q';
    }
    copy = board->piece_bb[B_QUEEN];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'q';
    }
    copy = board->piece_bb[W_KING];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'K';
    }
    copy = board->piece_bb[B_KING];
    while ((idx = pop_1st_bit(&copy)) != -1) {
        string[idx] = 'k';
    }

    return string;
}

/* Prints the board */
void print_board(board_t* board) {
    /* create string representing the playing field */
    char* board_string = create_board_string(board);

    /* print the chessboard-string row for row */
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            /* print row number */
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }
            /* print piece */
            char piece = board_string[pos_to_idx(x, y)];
            /* in PURPLE if blacks piece */
            if (piece >= 97 && piece <= 122) {
                fprintf(stderr, "%s%c%s  ", Color_PURPLE, piece, Color_END);
            }
            /* in WHITE if whites piece */
            else {
                fprintf(stderr, "%c  ", piece);
            }
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN,
            Color_END);
    fprintf(stderr, "\n");
    free(board_string);
}

/* Prints a bitboard 0/1 */
void print_bitboard(bitboard_t board) {
    char* string = (char*)malloc(64);
    for (int i = 0; i < 64; i++) {
        string[i] = '-';
    }

    int idx;
    while ((idx = pop_1st_bit(&board)) != -1) {
        string[idx] = 'o';
    }

    /* print the chessboard-string row for row */
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }
            char field = string[pos_to_idx(x, y)];
            fprintf(stderr, "%c  ", field);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN,
            Color_END);
    fprintf(stderr, "\n");
    free(string);
}

/* Prints move */
void print_move(move_t* move) {
    char* start_field = FIELD[move->from];
    char* end_field = FIELD[move->to];
    if (move->flags >= 8) {
        fprintf(stderr, "%s%.2s-%.2s-P%s", Color_PURPLE, start_field, end_field,
                Color_END);

    } else if (move->flags == KCASTLE || move->flags == QCASTLE) {
        fprintf(stderr, "%s%.2s-%.2s-C%s", Color_PURPLE, start_field, end_field,
                Color_END);
    } else {
        fprintf(stderr, "%s%.2s-%.2s%s", Color_PURPLE, start_field, end_field,
                Color_END);
    }
}

/* Prints move in LAN notation */
void print_LAN_move(move_t* move, player_t color_playing) {
    char* start_field = FIELD[move->from];
    char* end_field = FIELD[move->to];

    /* if promotion move */
    if (move->flags >= 8) {
        flag_t prom_flag = move->flags & (0b11);
        if (prom_flag == 0) {
            if (color_playing == WHITE) {
                printf("%.2s%.2sN", start_field, end_field);
            } else {
                printf("%.2s%.2sn", start_field, end_field);
            }
        } else if (prom_flag == 1) {
            if (color_playing == WHITE) {
                printf("%.2s%.2sB", start_field, end_field);
            } else {
                printf("%.2s%.2sb", start_field, end_field);
            }
        } else if (prom_flag == 2) {
            if (color_playing == WHITE) {
                printf("%.2s%.2sR", start_field, end_field);
            } else {
                printf("%.2s%.2sr", start_field, end_field);
            }
        } else {
            if (color_playing == WHITE) {
                printf("%.2s%.2sQ", start_field, end_field);
            } else {
                printf("%.2s%.2sq", start_field, end_field);
            }
        }
    }
    /* if not a promotion move */
    else {
        printf("%.2s%.2s", start_field, end_field);
    }
}

/* Creates LAN move string */
char* get_LAN_move(move_t* move, player_t color_playing) {
    char* start_field = FIELD[move->from];
    char* end_field = FIELD[move->to];
    char* buffer = NULL;
    /* if promotion move */
    if (move->flags >= 8) {
        buffer = (char*)malloc(6);
        flag_t prom_flag = move->flags & (0b11);
        if (prom_flag == 0) {
            if (color_playing == WHITE) {
                snprintf(buffer, 6, "%.2s%.2sN", start_field, end_field);
            } else {
                snprintf(buffer, 6, "%.2s%.2sn", start_field, end_field);
            }
        } else if (prom_flag == 1) {
            if (color_playing == WHITE) {
                snprintf(buffer, 6, "%.2s%.2sB", start_field, end_field);
            } else {
                snprintf(buffer, 6, "%.2s%.2sb", start_field, end_field);
            }
        } else if (prom_flag == 2) {
            if (color_playing == WHITE) {
                snprintf(buffer, 6, "%.2s%.2sR", start_field, end_field);
            } else {
                snprintf(buffer, 6, "%.2s%.2sr", start_field, end_field);
            }
        } else {
            if (color_playing == WHITE) {
                snprintf(buffer, 6, "%.2s%.2sQ", start_field, end_field);
            } else {
                snprintf(buffer, 6, "%.2s%.2sq", start_field, end_field);
            }
        }
    }
    /* if not a promotion move */
    else {
        buffer = (char*)malloc(5);
        snprintf(buffer, 5, "%.2s%.2s", start_field, end_field);
    }

    return buffer;
}

/* Prints the (PV line) upto given depth */
void print_line(tt_t tt, board_t* board, int depth) {
    /* make a copy of the board */
    board_t* board_copy = copy_board(board);

    /* for all depth, probe best move from the transpostiotn table and print it
     */
    for (int d = depth; d > 0; d--) {
        move_t* best_move = tt_best_move(tt, board_copy);
        if (best_move == NULL) {
            /* this can happen but shouldn't (if hash entry overwritten) */
            printf("NULL ");
            free_board(board_copy);
            return;
        }
        print_LAN_move(best_move, board_copy->player);
        printf(" ");
        do_move(board_copy, best_move);
        free_move(best_move);
    }

    /* free the board */
    free_board(board_copy);

    return;
}
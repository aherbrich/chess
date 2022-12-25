#include "../include/chess.h"

///////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

/* clears the board */
void clear_board(board_t *board) {
    /* clears the playing field */
    for(int row = 0; row < 8; row++) 
        for (int col = 0; col < 8; col++)
            board->playing_field[pos_to_idx(row,col)] = EMPTY;

    /* reset player, en passant field/possible, castle rights and ply number */
    board->player = 0;
    board->ep_field = 0;
    board->ep_possible = FALSE;
    board->castle_rights = 0b1111;
    board->ply_no = 0;
}

/* Allocate memory for a empty board */
board_t* init_board() {
    board_t* board = (board_t*)malloc(sizeof(board_t));
    board->playing_field = (piece_t*)calloc(64, sizeof(piece_t));
    clear_board (board);
    return board;
}

/* Free memory of board */

void free_board(board_t* board) {
    free(board->playing_field);
    free(board);
}

board_t* copy_board(board_t* board) {
    board_t* copy = (board_t*)malloc(sizeof(board_t));
    copy->playing_field = (piece_t*)calloc(64, sizeof(piece_t));

    memcpy(copy->playing_field, board->playing_field, 64);

    copy->player = board->player;
    copy->ep_field = board->ep_field;
    copy->ep_possible = board->ep_possible;
    copy->castle_rights = board->castle_rights;

    return copy;
}

/* Load a game position based on FEN */
void load_by_FEN(board_t* board, char* FEN) {
    int row = 7;
    int col = 0;
    char* ptr = FEN;

    /* clear board before setting it from a FEN */
    clear_board(board);

    while ((row != 0) || (col != 8)) {
        switch (*ptr) {
            case 'p':
                board->playing_field[pos_to_idx(row, col)] = PAWN | BLACK;
                col++;
                break;
            case 'n':
                board->playing_field[pos_to_idx(row, col)] = KNIGHT | BLACK;
                col++;
                break;
            case 'b':
                board->playing_field[pos_to_idx(row, col)] = BISHOP | BLACK;
                col++;
                break;
            case 'r':
                board->playing_field[pos_to_idx(row, col)] = ROOK | BLACK;
                col++;
                break;
            case 'q':
                board->playing_field[pos_to_idx(row, col)] = QUEEN | BLACK;
                col++;
                break;
            case 'k':
                board->playing_field[pos_to_idx(row, col)] = KING | BLACK;
                col++;
                break;
            case 'P':
                board->playing_field[pos_to_idx(row, col)] = PAWN | WHITE;
                col++;
                break;
            case 'N':
                board->playing_field[pos_to_idx(row, col)] = KNIGHT | WHITE;
                col++;
                break;
            case 'B':
                board->playing_field[pos_to_idx(row, col)] = BISHOP | WHITE;
                col++;
                break;
            case 'R':
                board->playing_field[pos_to_idx(row, col)] = ROOK | WHITE;
                col++;
                break;
            case 'Q':
                board->playing_field[pos_to_idx(row, col)] = QUEEN | WHITE;
                col++;
                break;
            case 'K':
                board->playing_field[pos_to_idx(row, col)] = KING | WHITE;
                col++;
                break;
            case '/':
                row--;
                col = 0;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                col += (*ptr - '0');
                break;
        }
        ptr++;
    }

    ptr++;

    switch (*ptr) {
        case 'w':
            board->player = WHITE;
            break;
        case 'b':
            board->player = BLACK;
            break;
    }

    ptr += 2;

    board->castle_rights = 0;
    while (*ptr != ' ') {
        switch (*ptr) {
            case 'K':
                ptr++;
                board->castle_rights = board->castle_rights | SHORTSIDEW;
                break;
            case 'k':
                ptr++;
                board->castle_rights = board->castle_rights | SHORTSIDEB;
                break;
            case 'Q':
                ptr++;
                board->castle_rights = board->castle_rights | LONGSIDEW;
                break;
            case 'q':
                ptr++;
                board->castle_rights = board->castle_rights | LONGSIDEB;
                break;
            case '-':
                ptr++;
                break;
        }
    }
    ptr++;

    if (*ptr == '-') {
        board->ep_field = 0;
        board->ep_possible = FALSE;
        ptr += 2;
    } else {
        piece_t idx = 0;
        while (*ptr != ' ') {
            switch (*ptr) {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                    idx = idx + (*ptr - 'a');
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
        board->ep_possible = TRUE;
        board->ep_field = idx;
        ptr++;
    }

    return;
}
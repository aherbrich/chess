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
                board->playing_field[pos_to_idx(row, col)] = 3;
                ptr++;
                col++;
                break;
            case 'n':
                board->playing_field[pos_to_idx(row, col)] = 5;
                ptr++;
                col++;
                break;
            case 'b':
                board->playing_field[pos_to_idx(row, col)] = 9;
                ptr++;
                col++;
                break;
            case 'r':
                board->playing_field[pos_to_idx(row, col)] = 17;
                ptr++;
                col++;
                break;
            case 'q':
                board->playing_field[pos_to_idx(row, col)] = 33;
                ptr++;
                col++;
                break;
            case 'k':
                board->playing_field[pos_to_idx(row, col)] = 65;
                ptr++;
                col++;
                break;
            case 'P':
                board->playing_field[pos_to_idx(row, col)] = 2;
                ptr++;
                col++;
                break;
            case 'N':
                board->playing_field[pos_to_idx(row, col)] = 4;
                ptr++;
                col++;
                break;
            case 'B':
                board->playing_field[pos_to_idx(row, col)] = 8;
                ptr++;
                col++;
                break;
            case 'R':
                board->playing_field[pos_to_idx(row, col)] = 16;
                ptr++;
                col++;
                break;
            case 'Q':
                board->playing_field[pos_to_idx(row, col)] = 32;
                ptr++;
                col++;
                break;
            case 'K':
                board->playing_field[pos_to_idx(row, col)] = 64;
                ptr++;
                col++;
                break;
            case '/':
                ptr++;
                row--;
                col = 0;
                break;
            case '1':
                ptr++;
                col += 1;
                break;
            case '2':
                ptr++;
                col += 2;
                break;
            case '3':
                ptr++;
                col += 3;
                break;
            case '4':
                ptr++;
                col += 4;
                break;
            case '5':
                ptr++;
                col += 5;
                break;
            case '6':
                ptr++;
                col += 6;
                break;
            case '7':
                ptr++;
                col += 7;
                break;
            case '8':
                ptr++;
                col += 8;
                break;
        }
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
                    idx = idx + 0;
                    break;
                case 'b':
                    idx = idx + 1;
                    break;
                case 'c':
                    idx = idx + 2;
                    break;
                case 'd':
                    idx = idx + 3;
                    break;
                case 'e':
                    idx = idx + 4;
                    break;
                case 'f':
                    idx = idx + 5;
                    break;
                case 'g':
                    idx = idx + 6;
                    break;
                case 'h':
                    idx = idx + 7;
                    break;
                case '1':
                    idx = idx + (8 * 0);
                    break;
                case '2':
                    idx = idx + (8 * 1);
                    break;
                case '3':
                    idx = idx + (8 * 2);
                    break;
                case '4':
                    idx = idx + (8 * 3);
                    break;
                case '5':
                    idx = idx + (8 * 4);
                    break;
                case '6':
                    idx = idx + (8 * 5);
                    break;
                case '7':
                    idx = idx + (8 * 6);
                    break;
                case '8':
                    idx = idx + (8 * 7);
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

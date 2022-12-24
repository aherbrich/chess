#include "../include/chess.h"

///////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

/* Allocate memory for a empty board */
board_t* init_board() {
    board_t* board = (board_t*)malloc(sizeof(board_t));
    board->playingfield = (piece_t*)calloc(64, sizeof(piece_t));

    board->player = 0;
    board->epfield = 0;
    board->eppossible = FALSE;
    board->castlerights = 0b1111;
    board->plynr = 0;
    return board;
}

/* Free memory of board */

void free_board(board_t* board) {
    free(board->playingfield);
    free(board);
}

board_t* copy_board(board_t* board) {
    board_t* copy = (board_t*)malloc(sizeof(board_t));
    copy->playingfield = (piece_t*)calloc(64, sizeof(piece_t));

    memcpy(copy->playingfield, board->playingfield, 64);

    copy->player = board->player;
    copy->epfield = board->epfield;
    copy->eppossible = board->eppossible;
    copy->castlerights = board->castlerights;

    return copy;
}

/* Load a game position based on FEN */
void loadByFEN(board_t* board, char* FEN) {
    int row = 7;
    int col = 0;
    char* ptr = FEN;

    while ((row != 0) || (col != 8)) {
        switch (*ptr) {
            case 'p':
                board->playingfield[posToIdx(row, col)] = 3;
                ptr++;
                col++;
                break;
            case 'n':
                board->playingfield[posToIdx(row, col)] = 5;
                ptr++;
                col++;
                break;
            case 'b':
                board->playingfield[posToIdx(row, col)] = 9;
                ptr++;
                col++;
                break;
            case 'r':
                board->playingfield[posToIdx(row, col)] = 17;
                ptr++;
                col++;
                break;
            case 'q':
                board->playingfield[posToIdx(row, col)] = 33;
                ptr++;
                col++;
                break;
            case 'k':
                board->playingfield[posToIdx(row, col)] = 65;
                ptr++;
                col++;
                break;
            case 'P':
                board->playingfield[posToIdx(row, col)] = 2;
                ptr++;
                col++;
                break;
            case 'N':
                board->playingfield[posToIdx(row, col)] = 4;
                ptr++;
                col++;
                break;
            case 'B':
                board->playingfield[posToIdx(row, col)] = 8;
                ptr++;
                col++;
                break;
            case 'R':
                board->playingfield[posToIdx(row, col)] = 16;
                ptr++;
                col++;
                break;
            case 'Q':
                board->playingfield[posToIdx(row, col)] = 32;
                ptr++;
                col++;
                break;
            case 'K':
                board->playingfield[posToIdx(row, col)] = 64;
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

    board->castlerights = 0;
    while (*ptr != ' ') {
        switch (*ptr) {
            case 'K':
                ptr++;
                board->castlerights = board->castlerights | SHORTSIDEW;
                break;
            case 'k':
                ptr++;
                board->castlerights = board->castlerights | SHORTSIDEB;
                break;
            case 'Q':
                ptr++;
                board->castlerights = board->castlerights | LONGSIDEW;
                break;
            case 'q':
                ptr++;
                board->castlerights = board->castlerights | LONGSIDEB;
                break;
            case '-':
                ptr++;
                break;
        }
    }
    ptr++;

    if (*ptr == '-') {
        board->epfield = 0;
        board->eppossible = FALSE;
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
        board->eppossible = TRUE;
        board->epfield = idx;
        ptr++;
    }

    return;
}

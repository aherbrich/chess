#include <string.h>
#include "../include/zobrist.h"
#include "../include/chess.h"

/* Clears the board */
void clear_board(board_t* board) {
    /* clear the playing field */
    for(int i = 0; i < 64; i++){
        board->playingfield[i] = NO_PIECE;
    }

    for(int i = 0; i < NR_PIECES; i++){
        board->piece_bb[i] = 0;
    }

    /* reset player, en passant field/possible, castle rights and ply number */
    board->player = WHITE;

    for(int i = 0; i < MAXPLIES; i++){
        board->history[i].captured = NO_PIECE;
        board->history[i].castlerights = 0b1111;
        board->history[i].epsq = NO_SQUARE;
        board->history[i].fifty_move_counter = 0;
        board->history[i].full_move_counter = 0;
    }

    board->ply_no = 0;

    board->hash = calculate_zobrist_hash(board);
}

/* Copies given board */
board_t* copy_board(board_t* board) {
    board_t* copy = (board_t*)malloc(sizeof(board_t));

    /* copy the playing field */
    for(int i = 0; i < 64; i++){
        copy->playingfield[i] = board->playingfield[i];
    }

    for(int i = 0; i < NR_PIECES; i++){
        copy->piece_bb[i] = board->piece_bb[i];
    }

    /* copy player, en passant field/possible, castle rights and ply number */
    copy->player = board->player;

    for(int i = 0; i < MAXPLIES; i++){
        copy->history[i].captured = board->history[i].captured;
        copy->history[i].castlerights = board->history[i].castlerights;
        copy->history[i].epsq = board->history[i].epsq;
        copy->history[i].fifty_move_counter = board->history[i].fifty_move_counter;
        copy->history[i].full_move_counter = board->history[i].full_move_counter;
    }

    copy->ply_no = board->ply_no;

    copy->hash = board->hash;

    return copy;
}

/* Allocates memory and initializes empty board */
board_t* init_board() {
    board_t* board = (board_t*)malloc(sizeof(board_t));
    clear_board(board);
    return board;
}

/* Frees memory of board */
void free_board(board_t* board) { free(board); }

/* Load a game position based on FEN */
void load_by_FEN(board_t* board, char* FEN) {
    int row = 7;
    int col = 0;
    char buffer[1024];
    strcpy(buffer, FEN);
    char* ptr = buffer;
    uint64_t shift = 0;

    /* clear board before setting it from a FEN */
    clear_board(board);

    /* set bitboards */
    while ((row != 0) || (col != 8)) {
        switch (*ptr) {
            case 'p':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_PAWN] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_PAWN;
                col++;
                break;
            case 'n':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_KNIGHT] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_KNIGHT;
                col++;
                break;
            case 'b':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_BISHOP] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_BISHOP;
                col++;
                break;
            case 'r':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_ROOK] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_ROOK;
                col++;
                break;
            case 'q':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_QUEEN] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_QUEEN;
                col++;
                break;
            case 'k':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[B_KING] |= shift;
                board->playingfield[pos_to_idx(row, col)] = B_KING;
                col++;
                break;
            case 'P':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_PAWN] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_PAWN;
                col++;
                break;
            case 'N':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_KNIGHT] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_KNIGHT;
                col++;
                break;
            case 'B':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_BISHOP] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_BISHOP;
                col++;
                break;
            case 'R':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_ROOK] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_ROOK;
                col++;
                break;
            case 'Q':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_QUEEN] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_QUEEN;
                col++;
                break;
            case 'K':
                shift = (1ULL << pos_to_idx(row, col));
                board->piece_bb[W_KING] |= shift;
                board->playingfield[pos_to_idx(row, col)] = W_KING;
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

    /* set player at turn */
    switch (*ptr) {
        case 'w':
            board->player = WHITE;
            break;
        case 'b':
            board->player = BLACK;
            break;
    }

    ptr += 2;

    /* set castle rights */
    flag_t cr = 0;
    while (*ptr != ' ') {
        switch (*ptr) {
            case 'K':
                ptr++;
                cr |= SHORTSIDEW;
                break;
            case 'k':
                ptr++;
                cr |= SHORTSIDEB;
                break;
            case 'Q':
                ptr++;
                cr |= LONGSIDEW;
                break;
            case 'q':
                ptr++;
                cr |= LONGSIDEB;
                break;
            case '-':
                ptr++;
                break;
        }
    }
    board->history[0].castlerights = cr;
    ptr++;

    /* set enpassant field (if possible) */
    if (*ptr == '-') {
        board->history[0].epsq = NO_SQUARE;
        ptr += 2;
    } else {
        idx_t idx = 0;
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
       board->history[0].epsq = idx;
        ptr++;
    }

    /* set number of consecutive non-captures and non-pawn moves */
    char* tmp;
    char* fifty_count = strtok_r(ptr, " ", &tmp);
    board->history[0].fifty_move_counter = (uint16_t)atoi(fifty_count);

    /* set full move counter */
    char* full_move = strtok_r(NULL, " ", &tmp);
    board->history[0].full_move_counter = (uint8_t)atoi(full_move);

    board->hash = calculate_zobrist_hash(board);
    return;
}
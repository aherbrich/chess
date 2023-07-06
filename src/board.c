#include <string.h>

#include "../include/chess.h"

/* Checks if two boards are the same */
int is_same_board(board_t* board1, board_t* board2) {
    if (board1->all != board2->all) return 0;
    if (board1->white != board2->white) return 0;
    if (board1->black != board2->black) return 0;

    if (board1->whitepawns != board2->whitepawns) return 0;
    if (board1->whiteknights != board2->whiteknights) return 0;
    if (board1->whitebishops != board2->whitebishops) return 0;
    if (board1->whiterooks != board2->whiterooks) return 0;
    if (board1->whitequeens != board2->whitequeens) return 0;
    if (board1->whiteking != board2->whiteking) return 0;

    if (board1->blackpawns != board2->blackpawns) return 0;
    if (board1->blackknights != board2->blackknights) return 0;
    if (board1->blackbishops != board2->blackbishops) return 0;
    if (board1->blackrooks != board2->blackrooks) return 0;
    if (board1->blackqueens != board2->blackqueens) return 0;
    if (board1->blackking != board2->blackking) return 0;

    if (board1->ep_possible != board2->ep_possible) return 0;
    if (board1->ep_field != board2->ep_field) return 0;
    if (board1->castle_rights != board2->castle_rights) return 0;
    if (board1->player != board2->player) return 0;

    return 1;
}
/* Clears the board */
void clear_board(board_t* board) {
    /* clear the playing field */
    board->whitepawns = 0;
    board->whiteknights = 0;
    board->whitebishops = 0;
    board->whiterooks = 0;
    board->whitequeens = 0;
    board->whiteking = 0;

    board->blackpawns = 0;
    board->blackknights = 0;
    board->blackbishops = 0;
    board->blackrooks = 0;
    board->blackqueens = 0;
    board->blackking = 0;

    board->black = 0;
    board->white = 0;
    board->all = 0;

    /* reset player, en passant field/possible, castle rights and ply number */
    board->player = WHITE;
    board->ep_field = -1;
    board->ep_possible = FALSE;
    board->castle_rights = 0b1111;
    board->ply_no = 0;
    board->fifty_move_counter = 0;
    board->full_move_counter = 0;
}

/* Copies given board */
board_t* copy_board(board_t* board) {
    board_t* copy = (board_t*)malloc(sizeof(board_t));

    /* copy the playing field */
    copy->whitepawns = board->whitepawns;
    copy->whiteknights = board->whiteknights;
    copy->whitebishops = board->whitebishops;
    copy->whiterooks = board->whiterooks;
    copy->whitequeens = board->whitequeens;
    copy->whiteking = board->whiteking;

    copy->blackpawns = board->blackpawns;
    copy->blackknights = board->blackknights;
    copy->blackbishops = board->blackbishops;
    copy->blackrooks = board->blackrooks;
    copy->blackqueens = board->blackqueens;
    copy->blackking = board->blackking;

    copy->black = board->black;
    copy->white = board->white;
    copy->all = board->all;

    /* copy player, en passant field/possible, castle rights and ply number */
    copy->player = board->player;
    copy->ep_field = board->ep_field;
    copy->ep_possible = board->ep_possible;
    copy->castle_rights = board->castle_rights;
    copy->ply_no = board->ply_no;
    copy->fifty_move_counter = board->fifty_move_counter;
    copy->full_move_counter = board->full_move_counter;

    return copy;
}

/* Recovers old board state from a different (the old) board */
/* WARNING: Frees the old board */
void recover_board(board_t* board, board_t* old_board) {
    /* copy the playing field */
    board->whitepawns = old_board->whitepawns;
    board->whiteknights = old_board->whiteknights;
    board->whitebishops = old_board->whitebishops;
    board->whiterooks = old_board->whiterooks;
    board->whitequeens = old_board->whitequeens;
    board->whiteking = old_board->whiteking;

    board->blackpawns = old_board->blackpawns;
    board->blackknights = old_board->blackknights;
    board->blackbishops = old_board->blackbishops;
    board->blackrooks = old_board->blackrooks;
    board->blackqueens = old_board->blackqueens;
    board->blackking = old_board->blackking;

    board->black = old_board->black;
    board->white = old_board->white;
    board->all = old_board->all;

    /* copy player, en passant field/possible, castle rights and ply number */
    board->player = old_board->player;
    board->ep_field = old_board->ep_field;
    board->ep_possible = old_board->ep_possible;
    board->castle_rights = old_board->castle_rights;
    board->ply_no = old_board->ply_no;
    board->fifty_move_counter = old_board->fifty_move_counter;
    board->full_move_counter = old_board->full_move_counter;

    free(old_board);
}

/* Allocates memory and initializes empty board */
board_t* init_board() {
    board_t* board = (board_t*)malloc(sizeof(board_t));
    clear_board(board);
    return board;
}

/* Frees memory of board */
void free_board(board_t* board) { free(board); }

/* Updates the white, the black and the all bitboard */
void update_white_black_all_boards(board_t* board) {
    board->black = board->blackpawns | board->blackknights |
                   board->blackbishops | board->blackrooks |
                   board->blackqueens | board->blackking;
    board->white = board->whitepawns | board->whiteknights |
                   board->whitebishops | board->whiterooks |
                   board->whitequeens | board->whiteking;
    board->all = board->black | board->white;
}

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
                board->blackpawns |= shift;
                col++;
                break;
            case 'n':
                shift = (1ULL << pos_to_idx(row, col));
                board->blackknights |= shift;
                col++;
                break;
            case 'b':
                shift = (1ULL << pos_to_idx(row, col));
                board->blackbishops |= shift;
                col++;
                break;
            case 'r':
                shift = (1ULL << pos_to_idx(row, col));
                board->blackrooks |= shift;
                col++;
                break;
            case 'q':
                shift = (1ULL << pos_to_idx(row, col));
                board->blackqueens |= shift;
                col++;
                break;
            case 'k':
                shift = (1ULL << pos_to_idx(row, col));
                board->blackking |= shift;
                col++;
                break;
            case 'P':
                shift = (1ULL << pos_to_idx(row, col));
                board->whitepawns |= shift;
                col++;
                break;
            case 'N':
                shift = (1ULL << pos_to_idx(row, col));
                board->whiteknights |= shift;
                col++;
                break;
            case 'B':
                shift = (1ULL << pos_to_idx(row, col));
                board->whitebishops |= shift;
                col++;
                break;
            case 'R':
                shift = (1ULL << pos_to_idx(row, col));
                board->whiterooks |= shift;
                col++;
                break;
            case 'Q':
                shift = (1ULL << pos_to_idx(row, col));
                board->whitequeens |= shift;
                col++;
                break;
            case 'K':
                shift = (1ULL << pos_to_idx(row, col));
                board->whiteking |= shift;
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

    update_white_black_all_boards(board);

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
    board->castle_rights = 0;
    while (*ptr != ' ') {
        switch (*ptr) {
            case 'K':
                ptr++;
                board->castle_rights |= SHORTSIDEW;
                break;
            case 'k':
                ptr++;
                board->castle_rights |= SHORTSIDEB;
                break;
            case 'Q':
                ptr++;
                board->castle_rights |= LONGSIDEW;
                break;
            case 'q':
                ptr++;
                board->castle_rights |= LONGSIDEB;
                break;
            case '-':
                ptr++;
                break;
        }
    }
    ptr++;

    /* set enpassant field (if possible) */
    if (*ptr == '-') {
        board->ep_field = -1;
        board->ep_possible = FALSE;
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
        board->ep_possible = TRUE;
        board->ep_field = idx;
        ptr++;
    }

    /* set number of consecutive non-captures and non-pawn moves */
    char* tmp;
    char* fifty_count = strtok_r(ptr, " ", &tmp);
    board->fifty_move_counter = (uint16_t)atoi(fifty_count);

    /* set full move counter */
    char* full_move = strtok_r(NULL, " ", &tmp);
    board->full_move_counter = (uint8_t)atoi(full_move);

    return;
}
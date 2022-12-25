#include "../include/zobrist.h"

zobrist_t zobtable;
htentry_t *httable;

idx_t indexing(piece_t piece) {
    switch (piece) {
        case PAWN | WHITE:
            return 0;
        case KNIGHT | WHITE:
            return 1;
        case BISHOP | WHITE:
            return 2;
        case ROOK | WHITE:
            return 3;
        case QUEEN | WHITE:
            return 4;
        case KING | WHITE:
            return 5;

        case PAWN | BLACK:
            return 6;
        case KNIGHT | BLACK:
            return 7;
        case BISHOP | BLACK:
            return 8;
        case ROOK | BLACK:
            return 9;
        case QUEEN | BLACK:
            return 10;
        case KING | BLACK:
            return 11;
        default:
            return -1;
    }
}

uint64_t get64rand() {
    return (((uint64_t)rand() << 0) & 0x000000000000FFFFull) |
           (((uint64_t)rand() << 16) & 0x00000000FFFF0000ull) |
           (((uint64_t)rand() << 32) & 0x0000FFFF00000000ull) |
           (((uint64_t)rand() << 48) & 0xFFFF000000000000ull);
}

void init_zobrist() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (int piece = 0; piece < 12; piece++) {
                zobtable.hashvalue[x][y][piece] = get64rand();
            }
        }
    }
    for (int i = 0; i < 13; i++) {
        zobtable.hashflags[i] = get64rand();
    }
}

uint64_t zobrist(board_t *board) {
    int hash = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            piece_t piece = board->playing_field[pos_to_idx(x, y)];
            if (piece != EMPTY) {
                idx_t pieceidx = indexing(piece);
                hash ^= zobtable.hashvalue[x][y][pieceidx];
            }
        }
    }

    if (board->ep_possible) {
        hash ^= zobtable.hashflags[board->ep_field % 8];
    }

    if ((board->castle_rights & LONGSIDEW) != 0) {
        hash ^= zobtable.hashflags[8];
    }

    if ((board->castle_rights & SHORTSIDEW) != 0) {
        hash ^= zobtable.hashflags[9];
    }

    if ((board->castle_rights & LONGSIDEB) != 0) {
        hash ^= zobtable.hashflags[10];
    }

    if ((board->castle_rights & SHORTSIDEB) != 0) {
        hash ^= zobtable.hashflags[11];
    }

    if (board->player == BLACK) {
        hash ^= zobtable.hashflags[12];
    }

    return hash;
}

void init_hashtable() {
    httable = (htentry_t *)malloc(sizeof(htentry_t) * HTSIZE);
    for (int i = 0; i < HTSIZE; i++) {
        httable[i].flags = 0;
        httable[i].eval = 0;

        httable[i].depth = -1;
        httable[i].bestmove = NULL;
        httable[i].hash = 0;
    }
}

void clear_hashtable() {
    for (int i = 0; i < HTSIZE; i++) {
        httable[i].flags = 0;
        httable[i].eval = 0;

        httable[i].depth = -1;
        httable[i].bestmove = NULL;
        httable[i].hash = 0;
    }
}

void storeTableEntry(board_t *board, int8_t flags, int16_t value, move_t *move, int8_t depth) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    httable[key].flags = flags;
    httable[key].depth = depth;
    httable[key].eval = value;
    httable[key].hash = hash;
    free_move(httable[key].bestmove);
    httable[key].bestmove = copy_move(move);
}

void probeTableEntry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    *flags = httable[key].flags;
    *depth = httable[key].depth;
    *value = httable[key].eval;
    *move = copy_move(httable[key].bestmove);
}
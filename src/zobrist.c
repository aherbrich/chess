#include "../include/zobrist.h"

zobrist_t zob_table;
htentry_t **ht_table;

/* Helper function that computes an index into the Zobrist hashtable based on the piece on the board */
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

/* Helper function that generates a random 64-bit number */
uint64_t get64rand() {
    return (((uint64_t)rand() << 0) & 0x000000000000FFFFull) |
           (((uint64_t)rand() << 16) & 0x00000000FFFF0000ull) |
           (((uint64_t)rand() << 32) & 0x0000FFFF00000000ull) |
           (((uint64_t)rand() << 48) & 0xFFFF000000000000ull);
}

/* Initializes the global Zobrist table */
void init_zobrist() {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (int piece = 0; piece < 12; piece++) {
                zob_table.hash_value[x][y][piece] = get64rand();
            }
        }
    }
    for (int i = 0; i < 14; i++) {
        zob_table.hash_flags[i] = get64rand();
    }
}

/* Zobrist-hashes a board using the Zobrist table */
uint64_t zobrist(board_t *board) {
    int hash = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            piece_t piece = board->playing_field[pos_to_idx(x, y)];
            if (piece != EMPTY) {
                idx_t piece_idx = indexing(piece);
                hash ^= zob_table.hash_value[x][y][piece_idx];
            }
        }
    }

    if (board->ep_possible) {
        hash ^= zob_table.hash_flags[board->ep_field % 8];
    }

    if ((board->castle_rights & LONGSIDEW) != 0) {
        hash ^= zob_table.hash_flags[8];
    }

    if ((board->castle_rights & SHORTSIDEW) != 0) {
        hash ^= zob_table.hash_flags[9];
    }

    if ((board->castle_rights & LONGSIDEB) != 0) {
        hash ^= zob_table.hash_flags[10];
    }

    if ((board->castle_rights & SHORTSIDEB) != 0) {
        hash ^= zob_table.hash_flags[11];
    }

    if (board->player == BLACK) {
        hash ^= zob_table.hash_flags[12];
    }

    if (board->player == WHITE) {
        hash ^= zob_table.hash_flags[13];
    }

    return hash;
}

/* Initializes a global hashtable */
void init_hashtable() {
    ht_table = (htentry_t **)malloc(sizeof(htentry_t *) * HTSIZE);
    for (int i = 0; i < HTSIZE; i++) {
        ht_table[i] = NULL;
    }
    return;
}

/* Clears the global hashtable */
void clear_hashtable() {
    for (int i = 0; i < HTSIZE; i++) {
        while(ht_table[i]) {
            htentry_t *tmp = ht_table[i]->next;
            free_move(ht_table[i]->best_move);
            free(ht_table[i]);
            ht_table[i] = tmp;            
        }
    }
}

/* Stores key value pair in the global hashtable */
void store_hashtable_entry(board_t *board, int8_t flags, int16_t value, move_t *move, int8_t depth) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *new = NULL;
    /* if there is not entry, just create a new one ... */
    if(!ht_table[key]) {
        ht_table[key] = (htentry_t *) malloc (sizeof(htentry_t));
        new = ht_table[key];
    } else {
        /* otherwise, check if there already is an entry in the list with the same hash */
        htentry_t *cur = ht_table[key];
        htentry_t *prev = cur;
        while(cur) {
            /* if there is one, just update it ... */
            if(cur->hash == hash) {
                cur->flags = flags;
                cur->depth = depth;
                cur->eval = value;
                free_move(cur->best_move);
                cur->best_move = copy_move(move);
                return;
            }
            prev = cur;
            cur = cur->next;
        }
        /* ... otherwise, create a new one at the end */
        prev->next = (htentry_t *) malloc (sizeof(htentry_t));
        new = prev->next;
    }
    
    /* finally, fill the new one with the data */
    new->flags = flags;
    new->depth = depth;
    new->eval = value;
    new->hash = hash;
    new->best_move = copy_move(move);
    new->next = NULL;
    return;
}

/* Probes table entry from hashtable and returns 1, if the entry is found (otherwise 0) */
int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while(cur) {
        /* if there is one, return the values by reference copy and indicate by returning 1 */
        if(cur->hash == hash) {
            *flags = cur->flags;
            *depth = cur->depth;
            *value = cur->eval;
            *move = copy_move(cur->best_move);
            return (1);
        }
        cur = cur->next;
    }

    /* otherwise, return 0 */
    return (0);
}

/* Gets the best move from the hashtable for the board position (or NULL, if there is not one) */
move_t *get_best_move_from_hashtable(board_t* board) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while(cur) {
        /* if there is one, return a deep copy of the best move */
        if(cur->hash == hash) {
            return (copy_move(cur->best_move));
        }
        cur = cur->next;
    }

    /* otherwise, return NULL */
    return (NULL);
}

/* Gets the eval from the hashtable for the board position */
int get_eval_from_hashtable(board_t* board) {
    uint64_t hash = zobrist(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while(cur) {
        /* if there is one, return the value */
        if(cur->hash == hash) {
            return (cur->eval);
        }
        cur = cur->next;
    }

    /* otherwise, return 0 */
    return -16000;
}
#include "../include/zobrist.h"

#include "../include/chess.h"
#include "../include/prettyprint.h"

zobrist_t zobrist_table;
htentry_t **ht_table;

/* Initializes the global zobrist table */
void initialize_zobrist_table() {
    /* Initialize random 64 bit number for each piece on each square of the
     * board */
    for (int i = 0; i < 64; i++) {
        for (int piece = 0; piece < 12; piece++) {
            zobrist_table.piece_random64[piece][i] = random_uint64();
        }
    }
    /* Initialize random 64 bit number for every board flag */
    for (int i = 0; i < 14; i++) {
        zobrist_table.flag_random64[i] = random_uint64();
    }
}

/* Hashes a board using zobrist hashing */
uint64_t calculate_zobrist_hash(board_t *board) {
    uint64_t hash = 0ULL;

    bitboard_t pawns, knights, bishops, rooks, queens, king;

    /* hash the WHITE piece positions */

    pawns = board->whitepawns;
    knights = board->whiteknights;
    bishops = board->whitebishops;
    rooks = board->whiterooks;
    queens = board->whitequeens;
    king = board->whiteking;

    while (pawns) {
        hash ^= zobrist_table.piece_random64[0][pop_1st_bit(&pawns)];
    }
    while (knights) {
        hash ^= zobrist_table.piece_random64[1][pop_1st_bit(&knights)];
    }
    while (bishops) {
        hash ^= zobrist_table.piece_random64[2][pop_1st_bit(&bishops)];
    }
    while (rooks) {
        hash ^= zobrist_table.piece_random64[3][pop_1st_bit(&rooks)];
    }
    while (queens) {
        hash ^= zobrist_table.piece_random64[4][pop_1st_bit(&queens)];
    }
    while (king) {
        hash ^= zobrist_table.piece_random64[5][pop_1st_bit(&king)];
    }

    /* hash the BLACK piece positions */

    pawns = board->blackpawns;
    knights = board->blackknights;
    bishops = board->blackbishops;
    rooks = board->blackrooks;
    queens = board->blackqueens;
    king = board->blackking;

    while (pawns) {
        hash ^= zobrist_table.piece_random64[6][pop_1st_bit(&pawns)];
    }
    while (knights) {
        hash ^= zobrist_table.piece_random64[7][pop_1st_bit(&knights)];
    }
    while (bishops) {
        hash ^= zobrist_table.piece_random64[8][pop_1st_bit(&bishops)];
    }
    while (rooks) {
        hash ^= zobrist_table.piece_random64[9][pop_1st_bit(&rooks)];
    }
    while (queens) {
        hash ^= zobrist_table.piece_random64[10][pop_1st_bit(&queens)];
    }
    while (king) {
        hash ^= zobrist_table.piece_random64[11][pop_1st_bit(&king)];
    }

    /* hash the flags */
    if (board->ep_possible) {
        hash ^= zobrist_table.flag_random64[board->ep_field % 8];
    }

    if ((board->castle_rights & LONGSIDEW)) {
        hash ^= zobrist_table.flag_random64[8];
    }

    if ((board->castle_rights & SHORTSIDEW)) {
        hash ^= zobrist_table.flag_random64[9];
    }

    if ((board->castle_rights & LONGSIDEB)) {
        hash ^= zobrist_table.flag_random64[10];
    }

    if ((board->castle_rights & SHORTSIDEB)) {
        hash ^= zobrist_table.flag_random64[11];
    }

    if (board->player == BLACK) {
        hash ^= zobrist_table.flag_random64[12];
    }

    if (board->player == WHITE) {
        hash ^= zobrist_table.flag_random64[13];
    }

    return hash;
}

/* Allocates memeory for hashtable and initializes it */
void initialize_hashtable() {
    ht_table = (htentry_t **)malloc(sizeof(htentry_t *) * HTSIZE);
    for (int i = 0; i < HTSIZE; i++) {
        ht_table[i] = NULL;
    }
    return;
}

/* Clears the global hashtable */
void clear_hashtable() {
    for (int i = 0; i < HTSIZE; i++) {
        /* remove all (so possibly multiple) hash entries at index i */
        while (ht_table[i]) {
            htentry_t *tmp = ht_table[i]->next;
            free_move(ht_table[i]->best_move);
            free(ht_table[i]);
            ht_table[i] = tmp;
        }
    }
}

/* Determines how full the hash table is in permill */
int hashtable_full_permill() {
    uint64_t count = 0;
    for (int i = 0; i < HTSIZE; i++) {
        if (ht_table[i]) count++;
    }
    return ((int)((count * 1000) / HTSIZE));
}

/* Calculates memory usage of hashtable in bytes */
uint64_t get_memory_usage_hashtable_in_bytes() {
    uint64_t count = 0;
    for (int i = 0; i < HTSIZE; i++) {
        htentry_t *tmp = ht_table[i];
        /* 8 bytes for htentry pointer */
        count += sizeof(htentry_t *);
        while (tmp) {
            /* 32 bytes of htentry */
            count += sizeof(htentry_t);
            /* 6 bytes for move */
            count += sizeof(move_t);
            /* 10 bytes added due to 16 byte alignmemnt */
            count += 10;
            tmp = tmp->next;
        }
    }
    return count;
}

/* Stores entry in the global hashtable */
void store_hashtable_entry(board_t *board, int8_t flags, int16_t value,
                           move_t *move, int8_t depth) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *new = NULL;
    /* if there is no entry, just create a new one ... */
    if (!ht_table[key]) {
        ht_table[key] = (htentry_t *)malloc(sizeof(htentry_t));
        new = ht_table[key];
    } else {
        /* otherwise, check if there already is an entry in the list with the
         * same hash we ignore the insanley unlikely event that two different
         * boards have the same hash */
        int counter = 0;
        htentry_t *cur = ht_table[key];
        htentry_t *prev = cur;
        while (cur) {
            /* we want to limit bucket size to 4 (so that the hashtable doesnt
             * grow into infinity) if we checked 4 slots but found no free slots
             * nor an entry for that board: we REPLACE last entry in bucket */
            if (counter >= 3) {
                cur->flags = flags;
                cur->depth = depth;
                cur->eval = value;
                free_move(cur->best_move);
                cur->best_move = copy_move(move);
                cur->hash = hash;
                return;
            }
            /* if there exists an entry for the board
             * we UPDATE the entry if it has a higher/same depth ... */
            if (cur->hash == hash) {
                if (depth >= cur->depth) {
                    cur->flags = flags;
                    cur->depth = depth;
                    cur->eval = value;
                    free_move(cur->best_move);
                    cur->best_move = copy_move(move);
                }
                return;
            }
            prev = cur;
            cur = cur->next;
            counter++;
        }
        /* if there exists no entry for that board but we have a free slot
         * create a new entry at the end of list we CREATE a new entry */
        prev->next = (htentry_t *)malloc(sizeof(htentry_t));
        new = prev->next;
    }

    /* finally, fill the new entry with the data */
    new->flags = flags;
    new->depth = depth;
    new->eval = value;
    new->hash = hash;
    new->best_move = copy_move(move);
    new->next = NULL;
    return;
}

/* Gets the best move from the hashtable for the board position (or NULL, if
 * there isnt one) */
move_t *get_best_move_from_hashtable(board_t *board) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while (cur) {
        /* if there is one, return a deep copy of the best move */
        if (cur->hash == hash) {
            return (copy_move(cur->best_move));
        }
        cur = cur->next;
    }

    /* otherwise, return NULL */
    return (NULL);
}

/* Probes table entry from hashtable and returns 1, if the entry is found
 * (otherwise 0) */
int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value,
                        move_t **move, int8_t *depth) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while (cur) {
        /* if there is one, return the values by reference copy and indicate by
         * returning 1 */
        if (cur->hash == hash) {
            *flags = cur->flags;
            *depth = cur->depth;
            *value = cur->eval;
            *move = copy_move(cur->best_move);
            return 1;
        }
        cur = cur->next;
    }

    /* otherwise, return 0 */
    return 0;
}

/* Gets the eval from the hashtable for the board position */
int get_eval_from_hashtable(board_t *board) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while (cur) {
        /* if there is one, return the value */
        if (cur->hash == hash) {
            return (cur->eval);
        }
        cur = cur->next;
    }

    /* otherwise, return worst eval */
    return -16000;
}

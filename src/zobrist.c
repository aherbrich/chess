#include "../include/chess.h"
#include "../include/zobrist.h"
#include "../include/prettyprint.h"

zobrist_t zobrist_table;
htentry_t** ht_table;


/* Helper function that generates a random 64-bit number */
uint64_t get64rand() {
    uint64_t n = ((uint64_t)rand()<<48) ^ ((uint64_t)rand()<<35) ^ ((uint64_t)rand()<<22) ^
                ((uint64_t)rand()<< 9) ^ ((uint64_t)rand()>> 4);

    return n;
}

/* Initializes the global zobrist table */
void initialize_zobrist_table() {
    for (int i = 0; i < 64; i++) {
        for (int piece = 0; piece < 12; piece++) {
            zobrist_table.piece_random64[piece][i] = get64rand();
        }
    }
    for (int i = 0; i < 14; i++) {
        zobrist_table.flag_random64[i] = get64rand();
    }
}

/* Hashes a board using the Zobrist table */
uint64_t calculate_zobrist_hash(board_t *board) {
    uint64_t hash = 0ULL;

    bitboard_t pawns, knights, bishops, rooks, queens, king;

    /* hash WHITE piece positions */

    pawns = board->whitepawns;
    knights = board->whiteknights;
    bishops = board->whitebishops;
    rooks = board->whiterooks;
    queens = board->whitequeens;
    king = board->whiteking;

    while(pawns){
        hash ^= zobrist_table.piece_random64[0][pop_1st_bit(&pawns)];
    }
    while(knights){
        hash ^= zobrist_table.piece_random64[1][pop_1st_bit(&knights)];
    }
    while(bishops){
       hash ^= zobrist_table.piece_random64[2][pop_1st_bit(&bishops)];
    }
    while(rooks){
        hash ^= zobrist_table.piece_random64[3][pop_1st_bit(&rooks)];
    }
    while(queens){
        hash ^= zobrist_table.piece_random64[4][pop_1st_bit(&queens)];
    }
    while(king){
        hash ^= zobrist_table.piece_random64[5][pop_1st_bit(&king)];
    }


    /* hash BLACK piece positions */

    pawns = board->blackpawns;
    knights = board->blackknights;
    bishops = board->blackbishops;
    rooks = board->blackrooks;
    queens = board->blackqueens;
    king = board->blackking;

    while(pawns){
        hash ^= zobrist_table.piece_random64[6][pop_1st_bit(&pawns)];
    }
    while(knights){
        hash ^= zobrist_table.piece_random64[7][pop_1st_bit(&knights)];
    }
    while(bishops){
       hash ^= zobrist_table.piece_random64[8][pop_1st_bit(&bishops)];
    }
    while(rooks){
        hash ^= zobrist_table.piece_random64[9][pop_1st_bit(&rooks)];
    }
    while(queens){
        hash ^= zobrist_table.piece_random64[10][pop_1st_bit(&queens)];
    }
    while(king){
        hash ^= zobrist_table.piece_random64[11][pop_1st_bit(&king)];
    }

    /* hash flags */
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

/* Initializes a global hashtable */
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
    uint64_t hash = calculate_zobrist_hash(board);
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
            /* if there is one, replace if it has a higher/same depth ... */
            if(cur->hash == hash) {
                if(depth >= cur->depth){
                    cur->flags = flags;
                    cur->depth = depth;
                    cur->eval = value;
                    free_move(cur->best_move);
                    cur->best_move = copy_move(move);
                    cur->board = copy_board(board);
                }
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
    new->board = copy_board(board);
    new->next = NULL;
    return;
}

/* Gets the best move from the hashtable for the board position (or NULL, if there is not one) */
move_t *get_best_move_from_hashtable(board_t* board) {
    uint64_t hash = calculate_zobrist_hash(board);
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


/* Probes table entry from hashtable and returns 1, if the entry is found (otherwise 0) */
int get_hashtable_entry(board_t *board, int8_t *flags, int16_t *value, move_t **move, int8_t *depth) {
    uint64_t hash = calculate_zobrist_hash(board);
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
            return 1;
        }
        cur = cur->next;
    }

    /* otherwise, return 0 */
    return 0;
}

/* Gets the best move from the hashtable for the board position (or NULL, if there is not one) */
void print_move_and_board_from_hashtable(board_t* board) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % HTSIZE;

    htentry_t *cur = ht_table[key];
    /* search the list for the entry with the same hash */
    while(cur) {
        /* if there is one, return a deep copy of the best move */
        if(cur->hash == hash) {
            print_board(board);
            printf("Real board with hash: \t%llu\n", hash);
            print_board(cur->board);
            printf("TT board with hash: \t%llu\n", calculate_zobrist_hash(cur->board));
            print_LAN_move(cur->best_move, cur->board->player);
            return;
        }
        cur = cur->next;
    }

    /* otherwise, return NULL */
    printf("No move found!\n");
}
#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"
#include "../include/database.h"

databaseentry_t** database;

/* Allocates memeory for database and initializes it */
void initialize_database() {
    database = (databaseentry_t **) malloc(sizeof(databaseentry_t *) * DATABASESIZE);
    for (int i = 0; i < DATABASESIZE; i++) {
        database[i] = NULL;
    }
    return;
}

/* Stores board positions and corresponding winrate in global database */
void update_database_entry(board_t *board, int winner) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % DATABASESIZE;

    databaseentry_t *new = NULL;
    // if there is no entry at key, just create a new one ... 
    if(!database[key]) {
        database[key] = (databaseentry_t *) malloc (sizeof(databaseentry_t));
        new = database[key];
    } else {
        // otherwise, check if there already is an entry in the list with the same hash 
        // we ignore the insanley unlikely event that two different boards have the same hash
        databaseentry_t* cur = database[key];
        databaseentry_t* prev = cur;
        while(cur) {
            // if there exists an entry for the board, update winrate
            if(cur->hash == hash) {
                cur->seen++;
                if(winner == WINWHITE) cur->white_won++;
                if(winner == WINBLACK) cur->black_won++;
                if(winner == DRAW) cur->draw++;
                return;
            }
            prev = cur;
            cur = cur->next;
        }
        // if there exists no entry for that board
        // we CREATE a new entry
        prev->next = (databaseentry_t *) malloc (sizeof(databaseentry_t));
        new = prev->next;
    }
    
    // finally, fill the new entry with the data 
    new->hash=hash;
    new->board=copy_board(board);
    new->next=NULL;

    new->seen=1;
    if(winner == WINWHITE) new->white_won = 1;
    if(winner == WINBLACK) new->black_won = 1;
    if(winner == DRAW) new->draw = 1;

    return;
}

/* Get the evaluation of a board */
void probe_database_entry(board_t* board) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % DATABASESIZE;

    databaseentry_t *cur = database[key];
    // search the list for the entry with the same hash 
    while(cur) {
        // if there is one, print eval
        if(cur->hash == hash) {
            fprintf(stderr, "\n#:\t%d\nW:\t%f\nB:\t%f\nD:\t%f\n", cur->seen, (float) cur->white_won/cur->seen, (float) cur->black_won/cur->seen, (float) cur->draw/cur->seen);
            return;
        }
        cur = cur->next;
    }

    fprintf(stderr, "\nNo entry found!\n");
    return;
}
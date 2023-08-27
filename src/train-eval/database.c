#include <stdio.h>

#include "include/train-eval/database.h"

#include "include/engine-core/board.h"
#include "include/engine-core/types.h"
#include "include/engine-core/zobrist.h"
#include "include/parse/parse.h"

/* global database*/
databaseentry_t **database;

/* ------------------------------------------------------------------------------------------------ */
/* functions and types for database                                                                 */
/* ------------------------------------------------------------------------------------------------ */

/* allocates memory and initiliazes database with NULL entries */
void initialize_database(void) {
    database =
        (databaseentry_t **)malloc(sizeof(databaseentry_t *) * DATABASESIZE);
    for (int i = 0; i < DATABASESIZE; i++) {
        database[i] = NULL;
    }
    return;
}

/* updates win percentage information i.e how often white/black won from this position*/
void update_database_entry(board_t *board, int winner) {
    uint64_t hash = calculate_zobrist_hash(board);
    uint64_t key = hash % DATABASESIZE;

    databaseentry_t *new = NULL;
    /* if there is no entry at key, just create a new one ... */
    if (!database[key]) {
        database[key] = (databaseentry_t *)malloc(sizeof(databaseentry_t));
        new = database[key];
    } else {
        /* otherwise, check if there already is an entry in the list with the
         * same hash we ignore the insanley unlikely event that two different
         * boards have the same hash */
        databaseentry_t *cur = database[key];
        databaseentry_t *prev = cur;
        while (cur) {
            /* if there exists an entry for the board, update winrate */
            if (cur->hash == hash) {
                cur->seen++;
                if (winner == WHITE_WIN) cur->white_won++;
                if (winner == BLACK_WIN) cur->black_won++;
                if (winner == DRAW) cur->draw++;
                return;
            }
            prev = cur;
            cur = cur->next;
        }
        /* if there exists no entry for that board
        we CREATE a new entry */
        prev->next = (databaseentry_t *)malloc(sizeof(databaseentry_t));
        new = prev->next;
    }

    /* finally, fill the new entry with the data */
    new->hash = hash;
    new->board = copy_board(board);
    new->next = NULL;

    new->seen = 1;
    if (winner == WHITE_WIN) new->white_won = 1;
    if (winner == BLACK_WIN) new->black_won = 1;
    if (winner == DRAW) new->draw = 1;

    return;
}

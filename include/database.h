#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "../include/chess.h"

typedef struct _databaseentry_t {
    uint64_t hash;
    board_t* board;
    struct _databaseentry_t *next;

    int seen;
    int white_won;
    int black_won;
    int draw;
} databaseentry_t;

extern databaseentry_t** database;

#define DATABASESIZE  18815231    // ~3.5GB  

/* Allocates memeory for database and initializes it */
void initialize_database();
/* Stores board positions and corresponding winrate in global database */
void update_database_entry(board_t *board, int winner);
/* Get the evaluation of a board */
void probe_database_entry(board_t* board);

#endif
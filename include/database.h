#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "../include/types.h"

#define DATABASESIZE 18815231

typedef struct _databaseentry_t {
    uint64_t hash;
    board_t* board;
    struct _databaseentry_t* next;

    int seen;
    int white_won;
    int black_won;
    int draw;
} databaseentry_t;

//////////////////////////////////////////////////////////////
//  FUNCTIONS
void initialize_database();
void update_database_entry(board_t* board, int winner);
void probe_database_entry(board_t* board);

#endif
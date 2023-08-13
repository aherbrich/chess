#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "include/engine-core/types.h"

#define DATABASESIZE 18815231

/* ------------------------------------------------------------------------------------------------ */
/* functions and types for database                                                                 */
/* ------------------------------------------------------------------------------------------------ */
typedef struct _databaseentry_t {
    uint64_t hash;                              /* hash of the board */
    board_t* board;                             /* pointer to the board */
    struct _databaseentry_t* next;              /* pointer to the next entry in the database */

    int seen;                                   /* number of times this board has been seen */
    int white_won;                              /* number of times white won */
    int black_won;                              /* number of times black won */
    int draw;                                   /* number of times the drawn */
} databaseentry_t;

/* allocates memory and initiliazes database with NULL entries */
void initialize_database();
/* updates win percentage information i.e how often white/black won from this position*/
void update_database_entry(board_t* board, int winner);

#endif
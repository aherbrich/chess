#ifndef __BOOK_H__
#define __BOOK_H__

#include "chess.h"
#include <stdint.h>

typedef struct _bookentry_t {
    int8_t possible;
    uint64_t hash;
    move_t* move;
} bookentry_t;

extern bookentry_t book[MAXNR_LINES][MAXDEPTH_LINE];

extern void init_book();
extern int book_possible(board_t* board);
extern move_t* getRandomBook(board_t* board);

#endif
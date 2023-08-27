#ifndef __MOVE_H__
#define __MOVE_H__

#include "include/engine-core/pq.h"
#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for move geneartion and execution                                                      */
/* ------------------------------------------------------------------------------------------------ */

/* generates all moves for a given board */
void generate_moves(board_t* board, maxpq_t* movelst);
/* executes a given move on a given board */
void do_move(board_t* board, move_t* move);
/* undoes a given move on a given board */
void undo_move(board_t* board, move_t* move);


/* ------------------------------------------------------------------------------------------------ */
/* helper functions for move handling & classification                                              */
/* ------------------------------------------------------------------------------------------------ */

/* returns a copy of a given move */
move_t* copy_move(move_t* move);
/* frees memory of a given move */
void free_move(move_t* move);
/* checks if two moves are the same */
int is_same_move(move_t* move1, move_t* move2);
/* checks if current player is in check - only use in right situations, see WARNING */
int is_in_check_fast(board_t* board);
/* checks if current player is in check, i.e. can only make check evading moves */
int is_in_check(board_t* board);


#endif

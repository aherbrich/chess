#ifndef __MOVE_H__
#define __MOVE_H__

#include "include/engine-core/pq.h"
#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for move geneartion and execution                                                      */
/* ------------------------------------------------------------------------------------------------ */

/* generates all moves for a given board */
void generate_moves(board_t* board, maxpq_t* movelst);
/* generates tactical moves for a given board */
void generate_tactical_moves(board_t* board, maxpq_t* movelst);
/* executes a given move on a given board */
void do_move(board_t* board, move_t move);
/* undoes a given move on a given board */
void undo_move(board_t* board, move_t move);
/* executes a null move */
void do_null_move(board_t* board);
/* undoes a null move */
void undo_null_move(board_t* board);


/* ------------------------------------------------------------------------------------------------ */
/* helper functions for move handling & classification                                              */
/* ------------------------------------------------------------------------------------------------ */

/* returns a copy of a given move */
move_t* copy_move(move_t* move);
/* frees memory of a given move */
void free_move(move_t* move);
/* checks if two moves are the same */
int is_same_move(move_t move1, move_t move2);
/* checks if current player is in check - only use in right situations, see WARNING */
int is_in_check_fast(board_t* board);
/* checks if current player is in check, i.e. can only make check evading moves */
int is_in_check(board_t* board);


/* ------------------------------------------------------------------------------------------------ */
/* functions made avaiable for SEE                                                                  */
/* ------------------------------------------------------------------------------------------------ */
/* returns bitboard of squares of pieces which can capture on given square as specified player */
bitboard_t attackers_from_both_sides(board_t *board, square_t sq, bitboard_t occ);
/* returns bitboard of squares of pieces which can capture on given square irrespective of piece color 
   and which lie on the line of the two squares */
bitboard_t consider_xray(board_t *board, square_t cap_sq, square_t from_sq, bitboard_t occ);


#endif

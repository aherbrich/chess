#ifndef __MOVEGEN_H__
#define __MOVEGEN_H__

#include "../include/types.h"
#include "../include/pq.h"

//  HELPER FUNCTIONS
void initialize_chess_engine_necessary();

//  MOVE GENERATION
int is_in_check(board_t* board);
int is_in_check_opponent(board_t* board);
void generate_moves(board_t* board, maxpq_t* movelst);

//  MOVE & EXECUTION
move_t* copy_move(move_t* move);
void free_move(move_t* move);
void do_move(board_t* board, move_t* move);
void undo_move(board_t *board, move_t* move);
int is_same_move(move_t* move1, move_t* move2);


#endif
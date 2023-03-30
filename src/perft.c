

#include "../include/chess.h"

/////////////////////////////////////////////////////////////
// PERFT TESTER

uint64_t move_gen(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);
    
    move_t* move;

    uint64_t num_positions = 0;


    while ((move = pop_max(&movelst)) != NULL) {
        do_move(board, move);
        num_positions += move_gen(board, depth - 1);
        undo_move(board);
        free_move(move);
    }
    return num_positions;
}
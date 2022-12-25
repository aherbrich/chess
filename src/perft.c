#include "../include/chess.h"

/////////////////////////////////////////////////////////////
// PERFT TESTER

int move_gen(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    node_t* movelst = generate_moves(board);
    move_t* move;

    int numPositions = 0;

    player_t playerwhomademove = board->player;

    while ((move = pop(movelst)) != NULL) {
        play_move(board, move, playerwhomademove);
        numPositions += move_gen(board, depth - 1);
        reverse_move(board, move, playerwhomademove);
        free_move(move);
    }
    free(movelst);
    return numPositions;
}
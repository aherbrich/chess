#include "../include/chess.h"

/////////////////////////////////////////////////////////////
// PERFT TESTER

int move_gen(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    node_t* move_list = generate_moves(board);
    move_t* move;

    int numPositions = 0;

    player_t player_who_made_move = board->player;

    while ((move = pop(move_list)) != NULL) {
        play_move(board, move, player_who_made_move);
        numPositions += move_gen(board, depth - 1);
        reverse_move(board, move, player_who_made_move);
        free_move(move);
    }
    free(move_list);
    return numPositions;
}
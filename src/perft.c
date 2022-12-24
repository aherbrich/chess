#include "../include/chess.h"

/////////////////////////////////////////////////////////////
// PERFT TESTER

int MoveGen(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    node_t* movelst = generateMoves(board);
    move_t* move;

    int numPositions = 0;

    player_t playerwhomademove = board->player;

    while ((move = pop(movelst)) != NULL) {
        playMove(board, move, playerwhomademove);
        numPositions += MoveGen(board, depth - 1);
        reverseMove(board, move, playerwhomademove);
        free_move(move);
    }
    free(movelst);
    return numPositions;
}
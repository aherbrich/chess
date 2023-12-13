#include "include/engine-core/engine.h"

/*
 * MAIN ENTRY POINT
 */
int main(void) {
    board_t* board = init_board();
    load_by_FEN(board,
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    initialize_attack_boards();
    initialize_helper_boards();
    initialize_zobrist_table();

    perft_divide(board, 5);

    free(board);

    return 0;
}

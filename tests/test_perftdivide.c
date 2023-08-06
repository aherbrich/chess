#include "../include/chess.h"

board_t* OLDSTATE[MAXPLIES];
uint64_t HISTORY_HASHES[MAXPLIES];

/*
 * MAIN ENTRY POINT
 */
int main() {
    board_t* board = init_board();
    load_by_FEN(board,
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    initialize_chess_engine_only_necessary();

    perft_divide(board, 5);

    free(board);
}
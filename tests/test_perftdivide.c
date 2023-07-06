#include "../include/chess.h"
#include "../include/prettyprint.h"

board_t* OLDSTATE[2048];
uint64_t HISTORY_HASHES[2048];

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
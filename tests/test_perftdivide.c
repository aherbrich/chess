#include "include/engine-core/board.h"
#include "include/engine-core/move.h"
#include "include/engine-core/perft.h"
#include "include/engine-core/types.h"
#include "include/ordering/urgencies.h"

/*
 * MAIN ENTRY POINT
 */
int main() {
    board_t* board = init_board();
    load_by_FEN(board,
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    initialize_chess_engine_necessary();

    perft_divide(board, 5);

    free(board);
    deletes_ht_urgencies(ht_urgencies);

    return 0;
}
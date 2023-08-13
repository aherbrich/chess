#include <stdio.h>

#include "include/engine-core/board.h"
#include "include/engine-core/move.h"
#include "include/engine-core/prettyprint.h"
#include "include/engine-core/types.h"
#include "include/ordering/urgencies.h"

/*
 * MAIN ENTRY POINT
 */
int main() {
    board_t* board = init_board();
    load_by_FEN(board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    initialize_chess_engine_necessary();

    maxpq_t move_lst;
    initialize_maxpq(&move_lst);
    generate_moves(board, &move_lst);

    move_t* move;
    printf("[");
    while ((move = pop_max(&move_lst))) {
        printf("\"");
        print_move_test(board, move);
        printf("\"");
        if ((&move_lst)->nr_elem == 0) {
            free_move(move);
            break;
        }
        printf(",");
        free_move(move);
    }
    printf("]");

    free(board);
    free_pq(&move_lst);
    deletes_ht_urgencies(ht_urgencies);
}
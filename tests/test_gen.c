#include "../include/chess.h"
#include "../include/prettyprint.h"

board_t* OLDSTATE[MAXPLIES];
uint64_t HISTORY_HASHES[MAXPLIES];

/*
 * MAIN ENTRY POINT
 */
int main() {
    board_t* board = init_board();
    load_by_FEN(board,
                "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    initialize_chess_engine_only_necessary();

    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    move_t* move;
    printf("[");
    while ((move = pop_max(&movelst))) {
        printf("\"");
        print_move_test(board, move);
        printf("\"");
        if((&movelst)->nr_elem == 0){
            break;
        }
        printf(",");
        free_move(move);
    }
    printf("]");

    free(board);
}
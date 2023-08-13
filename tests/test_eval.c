#include <stdio.h>
#include <sys/time.h>

#include "include/engine-core/engine.h"
#include "include/ordering/ordering.h"
#include "include/ordering/urgencies.h"

char STARTING_FEN[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
char TEST2_FEN[] =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
char TEST3_FEN[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
char TEST4_FEN[] =
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
char TEST5_FEN[] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
char TEST6_FEN[] =
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
char TEST7_FEN[] = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";

////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT
int main() {
    board_t* board = init_board();
    load_by_FEN(board, TEST7_FEN);

    initialize_chess_engine_necessary();
    initialize_hashtable();
    load_ht_urgencies_from_binary_file("ht_gaussians.bin", ht_urgencies);

    searchdata_t* search_data = init_search_data(board);

    clock_t end;
    clock_t begin;

    begin = clock();
    print_board(board);
    printf("\n");

    search_data->max_depth = 20;
    search_data->run_infinite = 1;
    search(search_data);
    int eval = search_data->best_eval;

    printf("\n");
    printf("Eval: \t\t%s%d%s\n", Color_PURPLE, eval, Color_END);
    printf("Best move:\t%s", Color_PURPLE);
    print_LAN_move(search_data->best_move, board->player);
    printf("\n%s", Color_END);
    end = clock();
    printf("\nTime: \t\t%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);

    free(board);
    free_search_data(search_data);
    deletes_ht_urgencies(ht_urgencies);

    return 0;
}
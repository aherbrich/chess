#include <stdio.h>

#include "include/engine-core/perft.h"

#include "include/engine-core/types.h"
#include "include/engine-core/move.h"
#include "include/engine-core/prettyprint.h"

/* Runs perft test for a given board and depth */
uint64_t perft(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    move_t* move;

    uint64_t num_positions = 0;

    while ((move = pop_max(&movelst)) != NULL) {
        do_move(board, move);
        num_positions += perft(board, depth - 1);
        undo_move(board, move);
        free_move(move);
    }
    return num_positions;
}

/* Runs perft divide test for a given board and depth */
uint64_t perft_divide(board_t* board, int depth) {
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    move_t* move;
    uint64_t all_nodes_count = 0;
    while ((move = pop_max(&movelst))) {
        do_move(board, move);
        uint64_t num_positions = perft(board, depth - 1);
        undo_move(board, move);

        print_LAN_move(move, board->player);
#ifdef __linux__
        printf(": %lu\n", num_positions);
#else
        printf(": %llu\n", num_positions);
#endif
        free_move(move);

        all_nodes_count += num_positions;
    }

#ifdef __linux__
    printf("\nNodes searched: %lu\n", all_nodes_count);
#else
    printf("\nNodes searched: %llu\n", all_nodes_count);
#endif

    return all_nodes_count;
}
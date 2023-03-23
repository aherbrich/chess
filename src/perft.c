

#include "../include/chess.h"

/////////////////////////////////////////////////////////////
// PERFT TESTER

int move_gen(board_t* board, int depth) {
    if (depth == 0) {
        return 1;
    }

    list_t* move_list = generate_pseudo_moves(board);
    move_t* move;

    int num_positions = 0;


    while ((move = pop(move_list)) != NULL) {
        // if(depth == 4){
        //     fprintf(stderr, "");
        //     print_move(move);
        //     fprintf(stderr, "\n");
        // }
        // if(depth == 3){
        //     fprintf(stderr, "\t");
        //     print_move(move);
        //     fprintf(stderr, "\n");
        // }

        do_move(board, move);
        num_positions += move_gen(board, depth - 1);
        undo_move(board, move);
        free_move(move);
    }
    free(move_list);
    // if(depth == 3){
    //     fprintf(stderr, "\t%d\n\n", num_positions);
    // }
    // if(depth == 2){
    //     fprintf(stderr, "\t\t%d\n\n", num_positions);
    // }
    return num_positions;
}
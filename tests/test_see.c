#include <stdio.h>

#include "include/engine-core/engine.h"

int main(void){
    /* initialize boards for movegen */
    initialize_attack_boards();
    initialize_helper_boards();

    /* initialize zobrist table */
    initialize_zobrist_table();

    /* intialize board */
    board_t* board = init_board();
    load_by_FEN(board, "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1");

    printf("board:\n");
    print_board(board);

    move_t move = {0, d3, e5, 0};
    int32_t swap_off_value = see(board, move);

    printf("SEE-value(");
    print_LAN_move(move, board->player);
    printf("): %d\n", swap_off_value);

    if(swap_off_value != -225){
        exit(EXIT_FAILURE);
    } else {
        printf("ALL GOOD!");
        exit(EXIT_SUCCESS);
    }
}
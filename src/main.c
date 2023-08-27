#include "include/engine-core/init.h"
#include "include/engine-core/types.h"
#include "include/engine-core/board.h"
#include "include/engine-core/uci.h"


#include <stdio.h>

/* MAIN ENTRY POINT */
int main(void) {
    /* initialize uci arguments */
    uci_args_t args = {
        .board = init_board(),
        .searchdata = NULL,
        .engine_info = init_engine_info(),
        .options = init_options(),
    };

    /* initialize chess engine */
    initialize_attack_boards();
    initialize_helper_boards();
    initialize_zobrist_table();
    initialize_eval_tables();

    /* start uci interface of chess engine */
    uci_interface_loop(&args);

    /* free board when finished */
    free_board(args.board);
}

#include <unistd.h>

#include "include/engine-core/init.h"
#include "include/engine-core/types.h"
#include "include/engine-core/board.h"
#include "include/engine-core/uci.h"


#include <stdio.h>

/* MAIN ENTRY POINT */
int main(int argc, char *argv[]) {
    /* variables set by command line options */
    int verbose = 0;

    /* command line parsing using getopt */
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Unknown option: %c\n", opt);
                exit(-1);
        }
    }

    /* output the options used */
    fprintf(stderr, "\033[0;35m");
    fprintf(stderr, "Settings:\n");
    fprintf(stderr, " Verbosity level: %s\n", (verbose) ? "high" : "low");
    fprintf(stderr, "\033[0m\n");

    /* initialize uci arguments */
    uci_args_t args = {
        .board = init_board(),
        .searchdata = NULL,
        .engine_info = init_engine_info(),
        .options = init_options(),
        .verbosity_level = verbose
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

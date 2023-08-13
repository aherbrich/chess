#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/engine-core/engine.h"
#include "include/ordering/ordering.h"
#include "include/ordering/urgencies.h"
#include "include/parse/parse.h"

/* main entry point of the program */
int main(int argc, char** argv) {
    int full_training = 0;
    int test_read_write_model = 0;

    /***** command line parsing using getopt *****/
    int c;
    while ((c = getopt(argc, argv, "fht")) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: %s\n", argv[0]);
                printf("Train the model for move ordering\n");
                printf("Options:\n");
                printf("  -h\t\t\tShow this help message\n");
                printf("  -t\t\t\tTest the write and load function for models\n");
                printf("  -f\t\t\tUse full factor graph training\n");
                exit(0);
            case 'f':
                full_training = 1;
                break;
            case 't':
                test_read_write_model = 1;
                break;
            default:
                fprintf(stderr, "Unknown option: %c\n", c);
                exit(-1);
        }
    }

    /* output the options used */
    printf("Options:\n");
    printf("\tFull Training: %s\n", (full_training) ? "yes" : "no");
    printf("\tTest model read-write: %s\n", (test_read_write_model) ? "yes" : "no");

    /****** parse chess game file ******/
    int nr_of_games = count_number_of_games();
    chessgame_t** chessgames = parse_chessgames_file(nr_of_games);

    /* initialize chess engine */
    initialize_chess_engine_necessary();
    initialize_move_zobrist_table();

    /****** train the model ******/
    train_info_t train_info = {
        .ht_urgencies = ht_urgencies,
        .prior = init_gaussian1D_standard_normal(),
        .beta = 0.5,
        .full_training = full_training,
        .base_filename = "snapshot",
        .verbosity = 1};
    train_model(chessgames, nr_of_games, train_info);

    /* write some output statistics about the model */
    fprintf(stderr, "Unique moves: %d\n", get_no_keys(ht_urgencies));

    /* write the mode to a binary file */
    write_ht_urgencies_to_binary_file("ht_urgencies.bin", ht_urgencies);

    /****** free chess games read in for the sake of training ******/
    for (int i = 0; i < nr_of_games; i++) {
        free(chessgames[i]->movelist);
        free(chessgames[i]);
    }
    free(chessgames);

    /****** test read-write code ******/
    if (test_read_write_model) {
        /* test that we have written the same model as we have in memory */
        urgency_ht_entry_t* ht_urgencies_test = initialize_ht_urgencies();
        load_ht_urgencies_from_binary_file("ht_urgencies.bin", ht_urgencies_test);

        /* compare the two tables */
        if (!ht_urgencies_equal(ht_urgencies, ht_urgencies_test)) {
            fprintf(stderr, "Error: ht_urgencies is not the same as ht_urgencies_test\n");
            exit(-1);
        } else {
            fprintf(stderr, "Hash tables are the same after write & read\n");
        }

        /* free the memory for the hash-table of urgencies */
        deletes_ht_urgencies(ht_urgencies_test);
    }
    /* free the memory for the hash-table of urgencies */
    deletes_ht_urgencies(ht_urgencies);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/engine.h"
#include "../include/factors.h"
#include "../include/gaussian.h"
#include "../include/ordering.h"
#include "../include/parse.h"

/* trains a Bayesian ranking model from the replay of the games */
void train_model(chessgame_t** chessgames, int nr_of_games, int full_training, const char* base_filename) {
    ranking_update_info_t *ranking_updates = NULL;
    int no_gaussian = 0, no_factors = 0;

    /* print some information on the screen */
    fprintf(stderr, "Training started (%s)\n", (full_training) ? "full" : "incremental");

    /* play games */
    for (int i = 0; i < nr_of_games; i++) {
        chessgame_t* chessgame = chessgames[i];
        board_t* board = init_board();
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        char* token = strtok(chessgame->movelist, " ");
        do {
            move_t* move = str_to_move(board, token);
            if (move) {
                /* genearate all possible moves */
                maxpq_t movelst;
                initialize_maxpq(&movelst);
                generate_moves(board, &movelst);

                /* create array to hold all indices of urgency beliefs corresponding to moves */
                int nr_of_moves = movelst.nr_elem;
                int urgencies_indices[nr_of_moves];
                int idx = 0;

                /* extract move ranking-info from MADE_MOVE*/
                urgencies_indices[idx] = calculate_order_hash(board, move);
                idx++;

                /* extract move ranking-info from OTHER_MOVES */
                move_t* other_move;
                while ((other_move = pop_max(&movelst)) != NULL) {
                    /* if we see MADE_MOVE, skip it */
                    if (is_same_move(move, other_move)) {
                        free_move(other_move);
                        continue;
                    }
                    /* else extract ranking-info from move */
                    urgencies_indices[idx] = calculate_order_hash(board, other_move);
                    idx++;
                    free_move(other_move);
                }

                // execute_ranking update
                if (full_training) {
                    ranking_updates = add_ranking_update_graph(ranking_updates, ht_urgencies, urgencies_indices, nr_of_moves, 0.5 * 0.5);
                    no_factors += (3 * nr_of_moves - 2);
                    no_gaussian += (8 * nr_of_moves - 5);
                } else {
                    update(ht_urgencies, urgencies_indices, nr_of_moves, 0.5 * 0.5);
                }

                /* execute MADE_MOVE */
                do_move(board, move);
                free_move(move);
                /* and continue with next (opponent) MADE_MOVE */
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token,
                        Color_END);
                exit(-1);
            }
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    /* now perform the full training and release the small update (factor) graphs */
    if (full_training) {
        fprintf(stderr, "Full training started\n\tNo. of factors: %d\n\tNo. of gaussians: %d\n", no_factors, no_gaussian);
        refresh_update_graph(ranking_updates, 1e-1, base_filename);
        delete_ranking_update_graphs(ranking_updates);
    }

    /* free chess games */
    for (int i = 0; i < nr_of_games; i++) {
        free(chessgames[i]->movelist);
        free(chessgames[i]);
    }
    free(chessgames);
}

int main(int argc, char** argv) {
    int full_training = 0;
    int test_read_write_model = 0;

    /* command line parsing using getopt */
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
    printf("\tFull Training: %s\n", (full_training)?"yes":"no");
    printf("\tTest model read-write: %s\n", (test_read_write_model)?"yes":"no");

    /* parse chess game file */
    int nr_of_games = count_number_of_games();
    chessgame_t** chessgames = parse_chessgames_file(nr_of_games);

    /* initialize chess engine */
    initialize_chess_engine_necessary();

    /* train the model */
    train_model(chessgames, nr_of_games, full_training, "snapshot");

    /* write the mode to a binary file */
    write_ht_urgencies_to_binary_file("ht_urgencies.bin", ht_urgencies);

    /* write some output statistics about the model */
    int count = 0;
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        if (mean(ht_urgencies[i]) != 0.0 || variance(ht_urgencies[i]) != 1.0) {
            count++;
        }
    }
    fprintf(stderr, "Unique moves: %d\n", count);

    if (test_read_write_model) {
        /* test that we have written the same model as we have in memory */
        gaussian_t* ht_urgencies_test = initialize_ht_urgencies();
        load_ht_urgencies_from_binary_file("ht_urgencies.bin", ht_urgencies_test);
        for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
            if (mean(ht_urgencies[i]) != mean(ht_urgencies_test[i]) ||
                variance(ht_urgencies[i]) != variance(ht_urgencies_test[i])) {
                fprintf(stderr, "Error: ht_urgencies[%d] is not the same\n", i);
                fprintf(stderr, "mean(ht_urgencies[%d]) = %f\n", i,
                        mean(ht_urgencies[i]));
                fprintf(stderr, "mean(ht_urgencies_test[%d]) = %f\n", i,
                        mean(ht_urgencies_test[i]));
                fprintf(stderr, "variance(ht_urgencies[%d]) = %f\n", i,
                        variance(ht_urgencies[i]));
                fprintf(stderr, "variance(ht_urgencies_test[%d]) = %f\n", i,
                        variance(ht_urgencies_test[i]));
                exit(-1);
            }
        }

        /* free the memory for the hash-table of urgencies */
        deletes_ht_urgencies(ht_urgencies_test);
    }

    /* free the memory for the hash-table of urgencies */
    deletes_ht_urgencies(ht_urgencies);

    return 0;
}
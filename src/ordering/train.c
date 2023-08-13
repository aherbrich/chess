#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/engine.h"
#include "../../include/factors.h"
#include "../../include/gaussian.h"
#include "../../include/ordering.h"
#include "../../include/parse.h"

/* trains a Bayesian ranking model from the replay of the games */
void train_model(chessgame_t** chessgames, int nr_of_games, train_info_t train_info) {
    /* parameters needed for full inference/training */
    ranking_update_info_t* ranking_updates = NULL;
    int no_gaussian = 0, no_factors = 0;

    /* print some information on the screen */
    if (train_info.verbosity >= 1) {
        fprintf(stderr, "Training started (%s)\n", (train_info.full_training) ? "full" : "incremental");
    }

    /* play games */
    for (int i = 0; i < nr_of_games; i++) {
        chessgame_t* chessgame = chessgames[i];
        board_t* board = init_board();
        load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        char* token = strtok(chessgame->movelist, " ");
        do {
            move_t* move = str_to_move(board, token);
            if (move) {
                /* generate all possible moves */
                maxpq_t movelst;
                initialize_maxpq(&movelst);
                generate_moves(board, &movelst);

                /* create array to hold all indices of urgency beliefs corresponding to moves */
                int nr_of_moves = movelst.nr_elem;
                gaussian_t* urgencies_ptr[nr_of_moves];
                int idx = 0;

                /* extract move ranking-info from MADE_MOVE*/
                int move_key = calculate_move_key(board, move);
                urgencies_ptr[idx] = get_urgency(train_info.ht_urgencies, move_key);
                if (urgencies_ptr[idx] == NULL) {
                    urgencies_ptr[idx] = add_urgency(train_info.ht_urgencies, move_key, train_info.prior);
                }
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
                    move_key = calculate_move_key(board, other_move);
                    urgencies_ptr[idx] = get_urgency(train_info.ht_urgencies, move_key);
                    if (urgencies_ptr[idx] == NULL) {
                        urgencies_ptr[idx] = add_urgency(train_info.ht_urgencies, move_key, train_info.prior);
                    }
                    idx++;

                    free_move(other_move);
                }

                // execute_ranking update
                if (train_info.full_training) {
                    ranking_updates = add_ranking_update_graph(ranking_updates, urgencies_ptr, nr_of_moves, train_info.beta * train_info.beta);
                    no_factors += (3 * nr_of_moves - 2);
                    no_gaussian += (8 * nr_of_moves - 5);
                } else {
                    update(urgencies_ptr, nr_of_moves, train_info.beta * train_info.beta);
                }

                /* execute MADE_MOVE */
                do_move(board, move);
                free_move(move);
                /* and continue with next (opponent) MADE_MOVE */
            } else {
                if (train_info.verbosity >= 1) {
                    print_board(board);
                    fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token, Color_END);
                }
                exit(-1);
            }
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    /* now perform the full training and release the small update (factor) graphs */
    if (train_info.full_training) {
        if (train_info.verbosity >= 1) {
            fprintf(stderr, "Full training started\n\tNo. of factors: %d\n\tNo. of Gaussian: %d\n", no_factors, no_gaussian);
        }
        refresh_update_graph(ranking_updates, 5e-1, train_info.base_filename);
        delete_ranking_update_graphs(ranking_updates);
    }

    return;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "include/engine-core/engine.h"
#include "include/ordering/factors.h"
#include "include/ordering/gaussian.h"
#include "include/ordering/ordering.h"
#include "include/ordering/urgencies.h"
#include "include/parse/parse.h"

/* test the trained model for k-fold cross validation */
double test_model(chess_game_t** chess_games, int no_games, int id) {
    int moves_played = 0;
    int moves_predicted_correctly = 0;

    /* seed random number generator */
    srand(time(NULL));

    /* make directory tmp if it doesn't exist */
    mkdir("tmp", 0777);

    char filename[100];
    snprintf(filename, 100, "tmp/output_test_%d.txt", id);
    FILE* file = fopen(filename, "w");

    /* play games */
    for (int i = 0; i < no_games; i++) {
        chess_game_t* chess_game = chess_games[i];
        board_t* board = init_board();
        load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        char* token = strtok(chess_game->move_list, " ");
        int move_nr = 0;
        do {
            move_t* move = str_to_move(board, token);
            if (move) {
                /* generate all possible moves */
                maxpq_t move_lst;
                initialize_maxpq(&move_lst);
                generate_moves(board, &move_lst);

                /* create arrays to hold move hashes and indices of moves as in move list */
                int nr_of_moves = move_lst.nr_elem;
                int move_keys[nr_of_moves];
                int move_indices[nr_of_moves];
                int idx = 0;

                /* calculate move hash for  MADE_MOVE*/
                move_keys[idx] = calculate_move_key(board, *move);
                move_indices[idx] = idx;
                idx++;

                /* calculate move hashes for OTHER_MOVES */
                move_t other_move;
                while (!is_empty(&move_lst)) {
                    other_move = pop_max(&move_lst);
                    /* if we see MADE_MOVE, skip it */
                    if (is_same_move(*move, other_move)) {
                        continue;
                    }
                    /* else determine hash */
                    move_keys[idx] = calculate_move_key(board, other_move);
                    move_indices[idx] = idx;
                    idx++;
                }

                /* extract means corresponding to moves */
                double means[nr_of_moves];
                for (int k = 0; k < nr_of_moves; k++) {
                    gaussian_t* g_ptr = get_urgency(ht_urgencies, move_keys[k]);
                    means[k] = mean((g_ptr) ? *g_ptr : init_gaussian1D_standard_normal());
                }

                /* sort means (and indices accordingly) */
                for (int k = 0; k < nr_of_moves; k++) {
                    for (int j = k + 1; j < nr_of_moves; j++) {
                        if (means[k] < means[j]) {
                            double tmp_mean = means[k];
                            means[k] = means[j];
                            means[j] = tmp_mean;

                            int tmp_idx = move_indices[k];
                            move_indices[k] = move_indices[j];
                            move_indices[j] = tmp_idx;
                        }
                    }
                }

                /* find position of MADE_MOVE in means array */
                int idx_of_made_move = 0;
                for (int k = 0; k < nr_of_moves; k++) {
                    if (move_indices[k] == 0) {
                        idx_of_made_move = k;
                        break;
                    }
                }

                /* find position of MADE_MOVE in legal move list */
                maxpq_t legals;
                initialize_maxpq(&legals);
                generate_moves(board, &legals);
                int idx_of_made_move_in_legal_moves = 0;
                while (!is_empty(&legals)) {
                    other_move = pop_max(&legals);
                    if (is_same_move(*move, other_move)) {
                        break;
                    }
                    idx_of_made_move_in_legal_moves++;
                }

                /* write results into file */
                /* current ply, rank by Bayesian, nr of possible, rank by random choice, rank by heuristic */
                fprintf(file, "%d %d %d %d %d\n", move_nr, idx_of_made_move, nr_of_moves, rand() % nr_of_moves, idx_of_made_move_in_legal_moves);

                /* check if MADE_MOVE has the highest mean */
                moves_played++;
                if (idx_of_made_move == 0) {
                    moves_predicted_correctly++;
                }

                /* execute MADE_MOVE */
                do_move(board, *move);
                free_move(move);

                /* and continue with next (opponent) MADE_MOVE */
                move_nr++;
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token, Color_END);
                exit(-1);
            }
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    fclose(file);

    /* return precision */
    return (float)moves_predicted_correctly / (float)moves_played;
}

/* runs a k-fold cross validation test */
double k_fold_cross_validation(chess_games_t chess_games, int no_folds) {
    int fold_size = chess_games.no_games / no_folds;
    int remainder = chess_games.no_games % no_folds;

    int start_idx = 0;
    int end_idx = fold_size;

    double total_accuracy = 0.0;

    for (int i = 0; i < no_folds; i++) {
        if (i == no_folds - 1) {
            end_idx += remainder;
        }

        /* create training set */
        int training_set_size = chess_games.no_games - (end_idx - start_idx);
        chess_game_t** training_set = (chess_game_t**)malloc(sizeof(chess_game_t*) * training_set_size);
        int idx = 0;
        for (int j = 0; j < chess_games.no_games; j++) {
            if (j < start_idx || j >= end_idx) {
                /* make deep copy of chess game */
                training_set[idx] = (chess_game_t*)malloc(sizeof(chess_game_t));
                training_set[idx]->move_list = (char*)malloc(strlen(chess_games.games[j]->move_list) + 1);
                strcpy(training_set[idx]->move_list, chess_games.games[j]->move_list);
                training_set[idx]->winner = chess_games.games[j]->winner;
                idx++;
            }
        }

        /* create test set */
        int test_set_size = end_idx - start_idx;
        chess_game_t** test_set = (chess_game_t**)malloc(sizeof(chess_game_t*) * test_set_size);
        idx = 0;
        for (int j = start_idx; j < end_idx; j++) {
            /* make deep copy of chess game */
            test_set[idx] = (chess_game_t*)malloc(sizeof(chess_game_t));
            test_set[idx]->move_list = (char*)malloc(strlen(chess_games.games[j]->move_list) + 1);
            strcpy(test_set[idx]->move_list, chess_games.games[j]->move_list);
            test_set[idx]->winner = chess_games.games[j]->winner;
            idx++;
        }

        /* train model */
        deletes_ht_urgencies(ht_urgencies);
        ht_urgencies = initialize_ht_urgencies();

        train_info_t train_info = {
            .ht_urgencies = ht_urgencies,
            .prior = init_gaussian1D_standard_normal(),
            .beta = 0.5,
            .full_training = 0,
            .base_filename = NULL,
            .verbosity = 1};
        train_model(training_set, training_set_size, train_info);

        printf("Training on fold %d done!\n", i + 1);

        /* output nr of keys */
        printf("Unique moves: %d\n", get_no_keys(ht_urgencies));

        /* test model */
        total_accuracy += test_model(test_set, test_set_size, i + 1) * (float)test_set_size / (float)chess_games.no_games;

        printf("Testing on fold %d  done!\n", i + 1);

        /* free sets */
        for (int j = 0; j < training_set_size; j++) {
            free(training_set[j]->move_list);
            free(training_set[j]);
        }
        for (int j = 0; j < test_set_size; j++) {
            free(test_set[j]->move_list);
            free(test_set[j]);
        }
        free(training_set);
        free(test_set);

        /* update indices */
        start_idx = end_idx;
        end_idx += fold_size;

        printf("Fold %d/%d done!\n", i + 1, no_folds);
        printf("Current prediction of overall accuracy: %f\n", total_accuracy * (float)no_folds / (float)(i + 1));
    }

    return total_accuracy;
}

int main(void) {
    /* parse chess game file */
    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn");
    chess_games_t chess_games = load_chess_games(file_name);

    /* initialize chess engine */
    initialize_attack_boards();
    initialize_helper_boards();
    initialize_zobrist_table();
    initialize_ranking_updates();
    ht_urgencies = initialize_ht_urgencies();
    initialize_move_zobrist_table();

    int folds = 10;

    double accuracy = k_fold_cross_validation(chess_games, folds);
    printf("%sAccuracy over %d folds:%s %f\n", Color_GREEN, folds, Color_END, accuracy);

    delete_chess_games(chess_games);
    deletes_ht_urgencies(ht_urgencies);

    return 0;
}

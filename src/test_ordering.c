#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/engine.h"
#include "../include/factors.h"
#include "../include/gaussian.h"
#include "../include/ordering.h"
#include "../include/parse.h"

/* Trains the model for k-fold cross validation */
void train_model(chessgame_t** chessgames, int nr_of_games) {
    deletes_ht_urgencies(ht_urgencies);
    ht_urgencies = initialize_ht_urgencies();

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

                /* create array to hold all indices of gaussians corresponding to moves a.k.a move hashes */
                int nr_of_moves = movelst.nr_elem;
                int move_hashes[nr_of_moves];
                int idx = 0;

                /* calculate move hash for MADE_MOVE*/
                move_hashes[idx] = calculate_order_hash(board, move);
                idx++;

                /* calculate move hashes for OTHER_MOVES */
                move_t* other_move;
                while ((other_move = pop_max(&movelst)) != NULL) {
                    /* if we see MADE_MOVE, skip it */
                    if (is_same_move(move, other_move)) {
                        free_move(other_move);
                        continue;
                    }
                    /* else determine hash  */
                    move_hashes[idx] = calculate_order_hash(board, other_move);
                    idx++;
                    free_move(other_move);
                }

                // execute_ranking update
                update(ht_urgencies, move_hashes, nr_of_moves, 0.5 * 0.5);

                /* execute MADE_MOVE */
                do_move(board, move);
                free_move(move);
                free_pq(&movelst);
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
}

/* Test the trained model for k-fold cross validation */
double test_model(chessgame_t** chessgames, int no_games, int id){
    int moves_played = 0;
    int moves_predicted_correctly = 0;

    /* seed random number generator */
    srand(time(NULL));

    /* make directory tmp if it doesnt exist */
    mkdir("tmp", 0777); 

    char filename[100];
    sprintf(filename, "tmp/output_test_%d.txt", id);
    FILE* file = fopen(filename, "w");

    /* play games */
    for (int i = 0; i < no_games; i++) {
        chessgame_t* chessgame = chessgames[i];
        board_t* board = init_board();
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        char* token = strtok(chessgame->movelist, " ");
        int move_nr = 0;
        do {
            move_t* move = str_to_move(board, token);
            if (move) {
                /* genearate all possible moves */
                maxpq_t movelst;
                initialize_maxpq(&movelst);
                generate_moves(board, &movelst);

                /* create arrays to hold move hashes and indices of moves as in movelist */
                int nr_of_moves = movelst.nr_elem;
                int move_hashes[nr_of_moves];
                int move_indices[nr_of_moves];
                int idx = 0;

                /* calculate move hash for  MADE_MOVE*/
                move_hashes[idx] = calculate_order_hash(board, move);
                move_indices[idx] = idx;
                idx++;

                /* calculate move hashes for OTHER_MOVES */
                move_t* other_move;
                while ((other_move = pop_max(&movelst)) != NULL) {
                    /* if we see MADE_MOVE, skip it */
                    if (is_same_move(move, other_move)) {
                        free_move(other_move);
                        continue;
                    }
                    /* else determine hash */
                    move_hashes[idx] = calculate_order_hash(board, other_move);
                    move_indices[idx] = idx;
                    idx++;
                    free_move(other_move);
                }

                /* extract means corresponding to moves */
                double means[nr_of_moves];
                for(int k = 0; k < nr_of_moves; k++){
                    means[k] = mean(ht_urgencies[move_hashes[k]]);
                }

                /* sort means (and indices accordingly) */
                for(int k = 0; k < nr_of_moves; k++){
                    for(int j = k + 1; j < nr_of_moves; j++){
                        if(means[k] < means[j]){
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
                for(int k = 0; k < nr_of_moves; k++){
                    if(move_indices[k] == 0){
                        idx_of_made_move = k;
                        break;
                    }
                }

                /* find position of MADE_MOVE in legal move list */
                maxpq_t legals;
                initialize_maxpq(&legals);
                generate_moves(board, &legals);
                int idx_of_made_move_in_legal_moves = 0;
                while((other_move = pop_max(&legals)) != NULL){
                    if(is_same_move(move, other_move)){
                        free_move(other_move);
                        break;
                    }
                    idx_of_made_move_in_legal_moves++;
                    free_move(other_move);
                }
                
                /* write results into file */
                /* current ply, rank by bayesian, nr of possible, rank by random choice, rank by heuristic */
                fprintf(file, "%d %d %d %d %d\n", move_nr, idx_of_made_move,  nr_of_moves, rand() % nr_of_moves, idx_of_made_move_in_legal_moves);

                /* check if MADE_MOVE has the highest mean */
                moves_played++;
                if(idx_of_made_move == 0){
                    moves_predicted_correctly++;
                }

                /* execute MADE_MOVE */
                do_move(board, move);
                free_move(move);
                free_pq(&movelst);
                free_pq(&legals);
                /* and continue with next (opponent) MADE_MOVE */
                move_nr++;
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token,
                        Color_END);
                exit(-1);
            }
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    fclose(file);
    /* return precision */
    return (float) moves_predicted_correctly / (float) moves_played;
}

/* Runs a k fold cross validation test */
double k_fold_cross_validation(chessgame_t** chessgames, int no_games, int no_folds){
    int fold_size = no_games / no_folds;
    int remainder = no_games % no_folds;

    int start_idx = 0;
    int end_idx = fold_size;

    double total_accuracy = 0.0;

    for(int i = 0; i < no_folds; i++){
        if(i == no_folds - 1){
            end_idx += remainder;
        }

        /* create training set */
        int training_set_size = no_games - (end_idx - start_idx);
        chessgame_t** training_set = (chessgame_t**)malloc(sizeof(chessgame_t*) * training_set_size);
        int idx = 0;
        for(int j = 0; j < no_games; j++){
            if(j < start_idx || j >= end_idx){
                /* make deep copy of chessgame */
                training_set[idx] = (chessgame_t*)malloc(sizeof(chessgame_t));
                training_set[idx]->movelist = (char*)malloc(strlen(chessgames[j]->movelist) + 1);
                strcpy(training_set[idx]->movelist, chessgames[j]->movelist);
                training_set[idx]->winner = chessgames[j]->winner;
                idx++;
            }
        }

        /* create test set */
        int test_set_size = end_idx - start_idx;
        chessgame_t** test_set = (chessgame_t**)malloc(sizeof(chessgame_t*) * test_set_size);
        idx = 0;
        for(int j = start_idx; j < end_idx; j++){
            /* make deep copy of chessgame */
            test_set[idx] = (chessgame_t*)malloc(sizeof(chessgame_t));
            test_set[idx]->movelist = (char*)malloc(strlen(chessgames[j]->movelist) + 1);
            strcpy(test_set[idx]->movelist, chessgames[j]->movelist);
            test_set[idx]->winner = chessgames[j]->winner;
            idx++;
        }

        /* train model */
        train_model(training_set, training_set_size);

        printf("Training on fold %d done!\n", i + 1);

        /* test model */
        total_accuracy += test_model(test_set, test_set_size, i+1) * (float) test_set_size/ (float) no_games;

        printf("Testing on fold %d  done!\n", i + 1);

        /* free sets */
        for(int j = 0; j < training_set_size; j++){
            free(training_set[j]->movelist);
            free(training_set[j]);
        }
        for(int j = 0; j < test_set_size; j++){
            free(test_set[j]->movelist);
            free(test_set[j]);
        }
        free(training_set);
        free(test_set);

        /* update indices */
        start_idx = end_idx;
        end_idx += fold_size;

        printf("Fold %d/%d done!\n", i + 1, no_folds);
        printf("Current prediction of overall accuracy: %f\n", total_accuracy * (float) no_folds / (float) (i + 1));
    }

    for(int i = 0; i < no_games; i++){
        free(chessgames[i]->movelist);
        free(chessgames[i]);
    }
    free(chessgames);

    return total_accuracy;
}


int main() {
    /* parse chess game file */
    int nr_of_games = count_number_of_games();
    chessgame_t** chessgames = parse_chessgames_file(nr_of_games);

    /* initialize chess engine */
    initialize_chess_engine_necessary();    

    int folds = 10;

    double accuracy =  k_fold_cross_validation(chessgames, nr_of_games, folds);

    printf("%sAccuracy over %d folds:%s %f\n",Color_GREEN, folds, Color_END, accuracy);

    return 0;
} 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/engine.h"
#include "../include/factors.h"
#include "../include/gaussian.h"
#include "../include/ordering.h"
#include "../include/parse.h"


void train_model(chessgame_t** chessgames, int nr_of_games) {
    initalize_ranking_updates();
    ht_gaussians = initialize_ht_gaussians(0.0, 1.0);

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

                /* create array to hold all indices of gaussians corresponding to moves */
                int nr_of_moves = movelst.nr_elem;
                int gaussian_indices[nr_of_moves];
                int idx = 0;

                /* extract move ranking-info from MADE_MOVE*/
                gaussian_indices[idx] = calculate_order_hash(board, move);
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
                    gaussian_indices[idx] = calculate_order_hash(board, other_move);
                    idx++;
                    free_move(other_move);
                }

                // execute_ranking update
                update(ht_gaussians, gaussian_indices, nr_of_moves, 0.5 * 0.5);

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

    /* free chess games */
    for (int i = 0; i < nr_of_games; i++) {
        free(chessgames[i]->movelist);
    }
    free(chessgames);
}

int main() {
    /* parse chess game file */
    int nr_of_games = count_number_of_games();
    chessgame_t** chessgames = parse_chessgames_file(nr_of_games);

    /* initialize chess engine */
    initialize_chess_engine_necessary();
    // printf("Moves made:\t%d\n", count_moves_made(chessgames, nr_of_games));

    train_model(chessgames, nr_of_games);

    int count = 0;
    for(int i = 0; i < HTSIZEGAUSSIAN; i++){
        printf("%.5f %.5f\n", mean(ht_gaussians[i]), variance(ht_gaussians[i]));
        if(mean(ht_gaussians[i]) != 0.0 || variance(ht_gaussians[i]) != 1.0){
            count++;
        }
    }
    fprintf(stderr, "Unique moves: %d\n", count);
    return 0;
}
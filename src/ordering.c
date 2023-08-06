#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/chess.h"
#include "../include/parse.h"
#include "../include/gaussian.h"
#include "../include/ordering.h"
#include "../include/san.h"
#include "../include/prettyprint.h"

#define HTSIZEGAUSSIAN 4608000

/* annoying externs which have to be in file */
board_t* OLDSTATE[MAXPLIES];
uint64_t HISTORY_HASHES[MAXPLIES];

int nodes_searched = 0;
int hash_used = 0;
int hash_bounds_adjusted = 0;
int pv_node_hit = 0;

/* table of gaussians corresponding to moves */
gaussian_t* ht_gaussians;

/* Initializes gaussian hashtable with standard normals */
gaussian_t* initialize_ht_gaussians(double mean, double var){
    gaussian_t* ht_table = (gaussian_t*) malloc(sizeof(gaussian_t) * HTSIZEGAUSSIAN);
    for (int i = 0; i < HTSIZEGAUSSIAN; i++) {
        ht_table[i] = init_gaussian1D_from_mean_and_variance(mean, var);
    }
    return ht_table;
}

/* Hash function from move to gaussian ht index */
/* NOTICE: Call with move ranking info struct */
int move_order_hash_by_infostruct(move_ranking_info_t* info){
    return info->piece_moved * 307200 + info->from * 4800 + info->piece_captured * 320 + info->to * 5 + info->piece_prom;
}

/* Hash function from move to gaussian ht index */
int move_order_hash(piece_t piece_moved, square_t from, piece_t piece_captured, square_t to, piece_t piece_prom){
    return piece_moved * 307200 + from * 4800 + piece_captured * 320 + to * 5 + piece_prom;
}

void train_model(chessgame_t** chessgames, int nr_of_games) {
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
                piece_t piece_prom = (move->flags & 0b1000) ? (move->flags & 0b11) : 4;
                gaussian_indices[idx] = move_order_hash(board->playingfield[move->from],
                                                            move->from,
                                                            board->playingfield[move->from],
                                                            move->from,
                                                            piece_prom);
                idx++;

                /* extract move ranking-info from OTHER_MOVES */
                move_t* other_move;
                while ((other_move = pop_max(&movelst)) != NULL) {
                    /* if we see MADE_MOVE, skip it */
                    if(is_same_move(move, other_move)){
                        free_move(other_move);
                        continue;
                    }
                    /* else extract ranking-info from move */
                    piece_prom = (other_move->flags & 0b1000) ? (other_move->flags & 0b11) : 4;
                    gaussian_indices[idx] = move_order_hash(board->playingfield[other_move->from],
                                                                other_move->from,
                                                                board->playingfield[other_move->from],
                                                                other_move->from,
                                                                piece_prom);
                    idx++;
                    free_move(other_move);
                }

                double beta_squared = 1.0;
                
                // execute_ranking_update(&ht_gaussians &gaussian_indices, nr_of_moves, beta_squared);

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
    initialize_chess_engine_only_necessary();
    printf("Moves made:\t%d\n", count_moves_made(chessgames, nr_of_games));
    
    train_model(chessgames, nr_of_games);
}
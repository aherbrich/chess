#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/chess.h"
#include "../include/factors.h"
#include "../include/gaussian.h"
#include "../include/ordering.h"
#include "../include/parse.h"
#include "../include/prettyprint.h"
#include "../include/san.h"

#define MAX_MOVES 512

/* statically allocate the factors to speed up execution */
gaussian_factor_info_t f[MAX_MOVES];
gaussian_mean_factor_info_t g[MAX_MOVES];
weighted_sum_factor_info_t s[MAX_MOVES];
greater_than_factor_info_t h[MAX_MOVES];

/* statically allocate the marginals and messages to speed up execution */
gaussian_t urgency[MAX_MOVES];
gaussian_t latent_urgency[MAX_MOVES];
gaussian_t diffs[MAX_MOVES];
gaussian_t msg_from_f_to_urgency[MAX_MOVES];
gaussian_t msg_from_g_to_latent_urgency[MAX_MOVES];
gaussian_t msg_from_g_to_urgency[MAX_MOVES];
gaussian_t msg_from_s_to_diffs[MAX_MOVES];
gaussian_t msg_from_s_to_top_urgency[MAX_MOVES];
gaussian_t msg_from_s_to_urgency[MAX_MOVES];
gaussian_t msg_from_h_to_diffs[MAX_MOVES];

/* this function should be called once and sets up the ranking update graph(s) */
void initalize_ranking_updates() {
    for (int i = 0; i < MAX_MOVES; ++i) {
        f[i].marginal = &urgency[i];
        f[i].msg = &msg_from_f_to_urgency[i];

        g[i].var_marginal = &latent_urgency[i];
        g[i].mean_marginal = &urgency[i];
        g[i].var_msg = &msg_from_g_to_latent_urgency[i];
        g[i].mean_msg = &msg_from_g_to_urgency[i];

        if (i < MAX_MOVES - 1) {
            s[i].a1 = 1.0;
            s[1].a2 = -1.0;
            s[i].s1_marginal = &latent_urgency[0];
            s[i].s2_marginal = &latent_urgency[i + 1];
            s[i].sum_marginal = &diffs[i];
            s[i].s1_msg = &msg_from_s_to_top_urgency[i];
            s[i].s2_msg = &msg_from_s_to_urgency[i];
            s[i].sum_msg = &msg_from_s_to_diffs[i];

            h[i].epsilon = 0.0;
            h[i].marginal = &diffs[i];
            h[i].msg = &msg_from_h_to_diffs[i];
        }
    }

    return;
}

/* updates the urgency belief distributions indexed by the no_hashes given */
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared) {
    assert(no_hashes <= MAX_MOVES);

    /* initialize all messages and marginals with the constant function */
    for (int i = 0; i < no_hashes; ++i) {
        urgency[i] = init_gaussian1D(0, 0);
        latent_urgency[i] = init_gaussian1D(0, 0);
        diffs[i] = init_gaussian1D(0, 0);
        msg_from_f_to_urgency[i] = init_gaussian1D(0, 0);
        msg_from_g_to_latent_urgency[i] = init_gaussian1D(0, 0);
        msg_from_g_to_urgency[i] = init_gaussian1D(0, 0);
        msg_from_s_to_diffs[i] = init_gaussian1D(0, 0);
        msg_from_s_to_top_urgency[i] = init_gaussian1D(0, 0);
        msg_from_s_to_urgency[i] = init_gaussian1D(0, 0);
        msg_from_h_to_diffs[i] = init_gaussian1D(0, 0);

        f[i].g = urgency_beliefs[hashes[i]];
        g[i].beta_squared = beta_squared;
    }

    /* run the schedule */
    for (int i = 0; i < no_hashes; ++i) {
        gaussian_factor_update(&f[i]);
        gaussian_mean_factor_update_to_variable(&g[i]);
    }

    double delta = 1e4;
    while (delta > 1e-6) {
        delta = 0.0;

        for (int i = 0; i < no_hashes - 1; ++i) {
            delta = fmax(delta, weighted_sum_factor_update_to_sum(&s[i]));
            delta = fmax(delta, greater_than_factor_update(&h[i]));
            delta = fmax(delta, weighted_sum_factor_update_to_summand1(&s[i]));
            delta = fmax(delta, weighted_sum_factor_update_to_summand2(&s[i]));
        }
    }

    for (int i = 0; i < no_hashes; ++i) {
        gaussian_mean_factor_update_to_mean(&g[i]);
        urgency_beliefs[hashes[i]] = urgency[i];
    }

    return;
}

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
gaussian_t* initialize_ht_gaussians(double mean, double var) {
    gaussian_t* ht_table = (gaussian_t*)malloc(sizeof(gaussian_t) * HTSIZEGAUSSIAN);
    for (int i = 0; i < HTSIZEGAUSSIAN; i++) {
        ht_table[i] = init_gaussian1D_from_mean_and_variance(mean, var);
    }
    return ht_table;
}

/* Hash function from move to gaussian ht index */
/* NOTICE: Call with move ranking info struct */
int move_order_hash_by_infostruct(move_ranking_info_t* info) {
    return info->piece_moved * 307200 + info->from * 4800 + info->piece_captured * 320 + info->to * 5 + info->piece_prom;
}

/* Hash function from move to gaussian ht index */
int move_order_hash(piece_t piece_moved, square_t from, piece_t piece_captured, square_t to, piece_t piece_prom) {
    return piece_moved * 307200 + from * 4800 + piece_captured * 320 + to * 5 + piece_prom;
}

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
                    if (is_same_move(move, other_move)) {
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
    initialize_chess_engine_only_necessary();
    // printf("Moves made:\t%d\n", count_moves_made(chessgames, nr_of_games));

    train_model(chessgames, nr_of_games);
}
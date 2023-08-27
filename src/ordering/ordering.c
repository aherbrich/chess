#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "include/ordering/ordering.h"

#include "include/engine-core/engine.h"
#include "include/ordering/factors.h"
#include "include/ordering/gaussian.h"
#include "include/ordering/urgencies.h"
#include "include/parse/parse.h"

#define EPSILON (1e-2)
#define MAX_ITER_CNT (10)

/* ------------------------------------------------------------------------------------------------ */
/* functions for a Zobrist hash                                                                     */
/* ------------------------------------------------------------------------------------------------ */

move_zobrist_table_t move_zobrist_table;

/* initializes the Zobrist hash for moves; should only be called once */
void initialize_move_zobrist_table(void) {
    srand(42);
    for (int i = 0; i < NR_PIECES; i++) {
        move_zobrist_table.piecefrom[i] = rand();
        move_zobrist_table.pieceto[i] = rand();
    }
    for (int i = 0; i < 64; i++) {
        move_zobrist_table.from[i] = rand();
        move_zobrist_table.to[i] = rand();
    }
    for (int i = 0; i < 5; i++) {
        move_zobrist_table.prompiece[i] = rand();
    }
    for (int i = 0; i < 2; i++) {
        move_zobrist_table.in_attack_range_after[i] = rand();
        move_zobrist_table.in_attack_range_before[i] = rand();
    }
}

/* hash function from move to urgencies hash-table index */
int calculate_move_key(board_t* board, move_t* move) {
    uint32_t key = 0;
    key ^= move_zobrist_table.piecefrom[board->playingfield[move->from]];
    key ^= move_zobrist_table.pieceto[board->playingfield[move->to]];
    key ^= move_zobrist_table.from[move->from];
    key ^= move_zobrist_table.to[move->to];
    key ^= move_zobrist_table.prompiece[(move->flags & 0b1000) ? (move->flags & 0b11) : 4];
    key ^= move_zobrist_table.in_attack_range_after[(board->attackmap & (1ULL << move->to)) != 0];
    key ^= move_zobrist_table.in_attack_range_before[(board->attackmap & (1ULL << move->from)) != 0];
    return key;
}

/* ------------------------------------------------------------------------------------------------ */
/* functions for online training of a Bayesian move ranking model                                   */
/* ------------------------------------------------------------------------------------------------ */

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
void initialize_ranking_updates(void) {
    for (int i = 0; i < MAX_MOVES; ++i) {
        f[i].marginal = &urgency[i];
        f[i].msg = &msg_from_f_to_urgency[i];

        g[i].var_marginal = &latent_urgency[i];
        g[i].mean_marginal = &urgency[i];
        g[i].var_msg = &msg_from_g_to_latent_urgency[i];
        g[i].mean_msg = &msg_from_g_to_urgency[i];

        if (i < MAX_MOVES - 1) {
            s[i].a1 = 1.0;
            s[i].a2 = -1.0;
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

/* updates the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
void update(gaussian_t** urgencies_ptr, int no_hashes, double beta_squared) {
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

        f[i].g = *urgencies_ptr[i];
        g[i].beta_squared = beta_squared;
    }

    /* run the schedule */
    for (int i = 0; i < no_hashes; ++i) {
        gaussian_factor_update(&f[i]);
        gaussian_mean_factor_update_to_variable(&g[i]);
    }

    int cnt = 0;
    double delta = EPSILON;
    while (delta >= EPSILON) {
        delta = 0;

        for (int i = 0; i < no_hashes - 1; ++i) {
            delta = fmax(delta, weighted_sum_factor_update_to_sum(&s[i]));
            delta = fmax(delta, greater_than_factor_update(&h[i]));
            delta = fmax(delta, weighted_sum_factor_update_to_summand1(&s[i]));
            delta = fmax(delta, weighted_sum_factor_update_to_summand2(&s[i]));
        }

        if (cnt++ > MAX_ITER_CNT) {
            fprintf(stderr, "\toverwriting convergence (delta: %f)\n", delta);
            break;
        }
    }

    for (int i = 0; i < no_hashes; ++i) {
        gaussian_mean_factor_update_to_mean(&g[i]);
        *urgencies_ptr[i] = urgency[i];
    }

    return;
}

/* ------------------------------------------------------------------------------------------------ */
/* functions for batch training of a Bayesian move ranking model                                    */
/* ------------------------------------------------------------------------------------------------ */

/* adds the factor graph that processes a single move made to the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
ranking_update_info_t* add_ranking_update_graph(ranking_update_info_t* root, gaussian_t** urgencies_ptr, int no_hashes, double beta_squared) {
    ranking_update_info_t* info = (ranking_update_info_t*)malloc(sizeof(ranking_update_info_t));

    /* copy the number of moves */
    info->no_moves = no_hashes;

    /* allocate memory for the factors */
    info->g = (gaussian_mean_factor_info_t*)malloc(no_hashes * sizeof(gaussian_mean_factor_info_t));
    info->s = (weighted_sum_factor_info_t*)malloc((no_hashes - 1) * sizeof(weighted_sum_factor_info_t));
    info->h = (greater_than_factor_info_t*)malloc((no_hashes - 1) * sizeof(greater_than_factor_info_t));

    info->latent_urgency = (gaussian_t*)malloc(no_hashes * sizeof(gaussian_t));
    info->msg_from_g_to_latent_urgency = (gaussian_t*)malloc(no_hashes * sizeof(gaussian_t));
    info->msg_from_g_to_urgency = (gaussian_t*)malloc(no_hashes * sizeof(gaussian_t));
    info->diffs = (gaussian_t*)malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_diffs = (gaussian_t*)malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_top_urgency = (gaussian_t*)malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_urgency = (gaussian_t*)malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_h_to_diffs = (gaussian_t*)malloc((no_hashes - 1) * sizeof(gaussian_t));

    /* wire the factors correctly */
    for (int i = 0; i < no_hashes; i++) {
        info->g[i].var_marginal = &info->latent_urgency[i];
        info->g[i].mean_marginal = urgencies_ptr[i];
        info->g[i].var_msg = &info->msg_from_g_to_latent_urgency[i];
        info->g[i].mean_msg = &info->msg_from_g_to_urgency[i];
        info->g[i].beta_squared = beta_squared;

        if (i < no_hashes - 1) {
            info->s[i].a1 = 1.0;
            info->s[i].a2 = -1.0;
            info->s[i].s1_marginal = &info->latent_urgency[0];
            info->s[i].s2_marginal = &info->latent_urgency[i + 1];
            info->s[i].sum_marginal = &info->diffs[i];
            info->s[i].s1_msg = &info->msg_from_s_to_top_urgency[i];
            info->s[i].s2_msg = &info->msg_from_s_to_urgency[i];
            info->s[i].sum_msg = &info->msg_from_s_to_diffs[i];

            info->h[i].epsilon = 0.0;
            info->h[i].marginal = &info->diffs[i];
            info->h[i].msg = &info->msg_from_h_to_diffs[i];
        }
    }

    /* initialize the messages and marginals correctly */
    for (int i = 0; i < no_hashes; i++) {
        info->latent_urgency[i] = init_gaussian1D(0, 0);
        info->msg_from_g_to_latent_urgency[i] = init_gaussian1D(0, 0);
        info->msg_from_g_to_urgency[i] = init_gaussian1D(0, 0);
        if (i < no_hashes - 1) {
            info->diffs[i] = init_gaussian1D(0, 0);
            info->msg_from_s_to_diffs[i] = init_gaussian1D(0, 0);
            info->msg_from_s_to_top_urgency[i] = init_gaussian1D(0, 0);
            info->msg_from_s_to_urgency[i] = init_gaussian1D(0, 0);
            info->msg_from_h_to_diffs[i] = init_gaussian1D(0, 0);
        }
    }

    /* make it the new root */
    info->next = root;
    return (info);
}

/* run message passing on the whole graph until convergence of epsilon; if base_filename is non-NULL then snapshots are stored after every iteration */
void refresh_update_graph(ranking_update_info_t* root, double epsilon, const char* base_filename) {
    double outer_delta = epsilon;

    while (outer_delta >= epsilon) {
        outer_delta = 0.0;
        struct timeval start;
        struct timeval end;
        gettimeofday(&start, 0);

        /* iterate over all ranking updates */
        ranking_update_info_t* ranking_update = root;
        while (ranking_update) {
            /* send message to latent urgencies */
            for (int i = 0; i < ranking_update->no_moves; ++i) {
                outer_delta = fmax(outer_delta, gaussian_mean_factor_update_to_variable(&ranking_update->g[i]));
                // fprintf(stderr, "   prior %d: %f +- %f\n", i, mean(*ranking_update->g[i].mean_marginal), sqrt(variance(*ranking_update->g[i].mean_marginal)));
            }

            /* iterate the ranking factors until convergence */
            double delta = EPSILON;
            while (delta >= EPSILON) {
                delta = 0.0;

                for (int i = 0; i < ranking_update->no_moves - 1; ++i) {
                    delta = fmax(delta, weighted_sum_factor_update_to_sum(&ranking_update->s[i]));
                    delta = fmax(delta, greater_than_factor_update(&ranking_update->h[i]));
                    delta = fmax(delta, weighted_sum_factor_update_to_summand1(&ranking_update->s[i]));
                    delta = fmax(delta, weighted_sum_factor_update_to_summand2(&ranking_update->s[i]));
                }
            }

            /* send message back to the actual urgencies */
            for (int i = 0; i < ranking_update->no_moves; ++i) {
                outer_delta = fmax(outer_delta, gaussian_mean_factor_update_to_mean(&ranking_update->g[i]));
                // fprintf(stderr, "   posterior %d: %f +- %f\n", i, mean(*ranking_update->g[i].mean_marginal), sqrt(variance(*ranking_update->g[i].mean_marginal)));
            }

            /* move to next update */
            ranking_update = ranking_update->next;
        }
        gettimeofday(&end, 0);
        double cpu_time_used = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0);

        fprintf(stderr, "\touter delta (%f seconds): %f\n", cpu_time_used, outer_delta);

        /* store the snapshot in a file */
        if (base_filename) {
            char filename[1024];
            snprintf(filename, 1024, "%s_%6.4f", base_filename, outer_delta);
            write_ht_urgencies_to_binary_file(filename, ht_urgencies);
        }
    }

    return;
}

/* deletes the linked list of ranking updates */
void delete_ranking_update_graphs(ranking_update_info_t* root) {
    ranking_update_info_t* next;
    while (root != NULL) {
        next = root->next;

        /* free all the factors and messages */
        free(root->g);
        free(root->s);
        free(root->h);
        free(root->latent_urgency);
        free(root->msg_from_g_to_latent_urgency);
        free(root->msg_from_g_to_urgency);
        free(root->diffs);
        free(root->msg_from_s_to_diffs);
        free(root->msg_from_s_to_top_urgency);
        free(root->msg_from_s_to_urgency);
        free(root->msg_from_h_to_diffs);

        /* free root itself */
        free(root);
        root = next;
    }
    return;
}

/* ------------------------------------------------------------------------------------------------ */
/* functions for making predictions based on the Bayesian move ranking model                        */
/* ------------------------------------------------------------------------------------------------ */

/* computes the probability of all moves being the most urgent move */
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared) {
    assert(no_hashes <= MAX_MOVES);

    for (int k = 0; k < no_hashes; ++k) {
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

            if (i == 0)
                f[i].g = urgency_beliefs[hashes[k]];
            else if (i == k)
                f[i].g = urgency_beliefs[hashes[0]];
            else
                f[i].g = urgency_beliefs[hashes[i]];
            g[i].beta_squared = beta_squared;
        }

        /* run the schedule */
        for (int i = 0; i < no_hashes; ++i) {
            gaussian_factor_update(&f[i]);
            gaussian_mean_factor_update_to_variable(&g[i]);
        }

        double delta = 1e4;
        while (delta > 1e-4) {
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

        /* reset all the marginals and compute the log-normalization constant */
        for (int i = 0; i < no_hashes; ++i) {
            urgency[i] = init_gaussian1D(0, 0);
            latent_urgency[i] = init_gaussian1D(0, 0);
            diffs[i] = init_gaussian1D(0, 0);
        }

        double logZ = 0.0;
        for (int i = 0; i < no_hashes; ++i) {
            logZ += gaussian_factor_log_variable_norm(&f[i]);
            logZ += gaussian_mean_factor_log_variable_norm(&g[i]);
            if (i < no_hashes - 1) {
                logZ += weighted_sum_factor_log_variable_norm(&s[i]);
                logZ += greater_than_factor_log_variable_norm(&h[i]);
            }
        }

        for (int i = 0; i < no_hashes; ++i) {
            logZ += gaussian_mean_factor_log_factor_norm(&g[i]);
            if (i < no_hashes - 1) {
                logZ += weighted_sum_factor_log_factor_norm(&s[i]);
                logZ += greater_than_factor_log_factor_norm(&h[i]);
            }
        }

        prob[k] = exp(logZ);
    }

    return;
}

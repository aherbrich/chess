#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "../../include/engine.h"
#include "../../include/factors.h"
#include "../../include/gaussian.h"
#include "../../include/ordering.h"
#include "../../include/parse.h"

#define EPSILON (1e-3)

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the hash-table of a urgencies for the Bayesian move ranking model         */
/* ------------------------------------------------------------------------------------------------ */

/* a unique cookie representing the file version */
int file_version_cookie = 0x10;

/* hash table of urgencies for each move (hash) */
gaussian_t* ht_urgencies = 0;

/* initializes an urgency hashtable with standard Normals */
gaussian_t* initialize_ht_urgencies() {
    gaussian_t* ht = (gaussian_t*)malloc(sizeof(gaussian_t) * HT_GAUSSIAN_SIZE);
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        ht[i] = init_gaussian1D_from_mean_and_variance(0, 1);
    }
    return ht;
}

/* deletes the memory for a hashtable of urgencies */
void deletes_ht_urgencies(gaussian_t* ht) {
    if (ht) free(ht);
    return;
}

/* hash function from move to urgencies hash-table index */
int calculate_order_hash(board_t* board, move_t* move) {
    piece_t piece_moved = board->playingfield[move->from];
    square_t from = move->from;
    piece_t piece_captured = board->playingfield[move->to];
    square_t to = move->to;
    piece_t piece_prom = (move->flags & 0b1000) ? (move->flags & 0b11) : 4;
    int in_attack_range = (board->attackmap & (1ULL << move->to)) != 0;

    return piece_moved * 614400 + from * 9600 + piece_captured * 640 + to * 10 + piece_prom * 2 + in_attack_range;
}
/* writes an urgencies hash-table to a file (only entries which are different from the prior) */
void write_ht_urgencies_to_binary_file(const char* file_name, const gaussian_t* ht) {
    FILE* fp = fopen(file_name, "wb");
    if (fp != NULL) {
        fwrite(&file_version_cookie, sizeof(int), 1, fp);
        for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
            if (ht[i].tau != 0 || ht[i].rho != 1) {
                fwrite(&i, sizeof(int), 1, fp);
                fwrite(&ht[i], sizeof(gaussian_t), 1, fp);
            }
        }
    }
    fclose(fp);
}

/* loads a hash-table of urgencies from a file (only entries which are different from the prior) */
void load_ht_urgencies_from_binary_file(const char* file_name, gaussian_t* ht) {
    FILE* fp = fopen(file_name, "rb");
    if (fp != NULL) {
        int cookie;
        fread(&cookie, sizeof(int), 1, fp);

        if (cookie != file_version_cookie) {
            fprintf(stderr, "Error: file version mismatch\n");
            exit(1);
        }

        int hash;
        gaussian_t gaussian;
        while (fread(&hash, sizeof(int), 1, fp) == 1) {
            fread(&gaussian, sizeof(gaussian_t), 1, fp);
            ht[hash] = gaussian;
        }
    }
    fclose(fp);
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
void initialize_ranking_updates() {
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

    double delta = EPSILON;
    while (delta >= EPSILON) {
        delta = 0;

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

/* ------------------------------------------------------------------------------------------------ */
/* functions for batch training of a Bayesian move ranking model                                    */
/* ------------------------------------------------------------------------------------------------ */

/* adds the factor graph that processes a single move made to the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
ranking_update_info_t* add_ranking_update_graph(ranking_update_info_t* root, gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared) {
    ranking_update_info_t* info = (ranking_update_info_t*) malloc(sizeof(ranking_update_info_t));

    /* copy the number of moves */
    info->no_moves = no_hashes;

    /* allocate memory for the factors */
    info->g = (gaussian_mean_factor_info_t*) malloc(no_hashes * sizeof(gaussian_mean_factor_info_t));
    info->s = (weighted_sum_factor_info_t*) malloc((no_hashes - 1) * sizeof(weighted_sum_factor_info_t));
    info->h = (greater_than_factor_info_t*) malloc((no_hashes - 1) * sizeof(greater_than_factor_info_t));

    info->latent_urgency = (gaussian_t*) malloc(no_hashes * sizeof(gaussian_t));
    info->msg_from_g_to_latent_urgency = (gaussian_t*) malloc(no_hashes * sizeof(gaussian_t));
    info->msg_from_g_to_urgency = (gaussian_t*) malloc(no_hashes * sizeof(gaussian_t));
    info->diffs = (gaussian_t*) malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_diffs = (gaussian_t*) malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_top_urgency = (gaussian_t*) malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_s_to_urgency = (gaussian_t*) malloc((no_hashes - 1) * sizeof(gaussian_t));
    info->msg_from_h_to_diffs = (gaussian_t*) malloc((no_hashes - 1) * sizeof(gaussian_t));

    /* wire the factors correctly */
    for (int i = 0; i < no_hashes; i++) {
        info->g[i].var_marginal = &info->latent_urgency[i];
        info->g[i].mean_marginal = &urgency_beliefs[hashes[i]];
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
        info->latent_urgency[i] = init_gaussian1D(0,0);
        info->msg_from_g_to_latent_urgency[i] = init_gaussian1D(0,0);
        info->msg_from_g_to_urgency[i] = init_gaussian1D(0,0);
        if (i < no_hashes - 1) {
            info->diffs[i] = init_gaussian1D(0,0);
            info->msg_from_s_to_diffs[i] = init_gaussian1D(0,0);
            info->msg_from_s_to_top_urgency[i] = init_gaussian1D(0,0);
            info->msg_from_s_to_urgency[i] = init_gaussian1D(0,0);
            info->msg_from_h_to_diffs[i] = init_gaussian1D(0,0);
        }
    }

    /* make it the new root */
    info->next = root;
    return (info);
}

/* run message passing on the whole graph until convergence of epsilon; if base_filename is non-NULL then snapshots are stored after every iteration */
void refresh_update_graph(ranking_update_info_t *root, double epsilon, const char* base_filename) {
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
        double cpu_time_used = ((end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec)/1000000.0);


        fprintf(stderr, "\touter delta (%f seconds): %f\n", cpu_time_used, outer_delta);

        /* store the snapshot in a file */
        if (base_filename) {
            char filename[1024];
            sprintf(filename, "%s_%6.4f", base_filename, outer_delta);
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
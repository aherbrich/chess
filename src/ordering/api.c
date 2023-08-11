#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/engine.h"
#include "../../include/factors.h"
#include "../../include/gaussian.h"
#include "../../include/ordering.h"
#include "../../include/parse.h"

/* a unique cookie representing the file version */
int file_version_cookie = 0x10;

/* global table of Gaussians corresponding to moves */
gaussian_t* ht_gaussians = NULL;

/* initializes a Gaussian hashtable with standard Normals */
gaussian_t* initialize_ht_gaussians() {
    gaussian_t* ht = (gaussian_t*)malloc(sizeof(gaussian_t) * HT_GAUSSIAN_SIZE);
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        ht[i] = init_gaussian1D_from_mean_and_variance(0, 1);
    }
    return ht;
}

/* deletes the memory for a hashtable of Gaussians */
void deletes_ht_gaussians(gaussian_t* ht) {
    if (ht) free(ht);
    return;
}

/* hash function from move to gaussian ht index */
int calculate_order_hash(board_t* board, move_t* move) {
    piece_t piece_moved = board->playingfield[move->from];
    square_t from = move->from;
    piece_t piece_captured = board->playingfield[move->to];
    square_t to = move->to;
    piece_t piece_prom = (move->flags & 0b1000) ? (move->flags & 0b11) : 4;

    return piece_moved * 307200 + from * 4800 + piece_captured * 320 + to * 5 + piece_prom;
}

/* writes a Gaussian hash-table to a file (only entries which are different from the prior) */
void write_ht_gaussians_to_binary_file(const char* file_name, const gaussian_t* ht) {
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

/* loads a Gaussian hash-table from a file (only entries which are different from the prior) */
void load_ht_gaussians_from_binary_file(const char* file_name, gaussian_t* ht) {
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
    while (delta > 1e-3) {
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

/* computes the probability of all moves being the most urgent moves */
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
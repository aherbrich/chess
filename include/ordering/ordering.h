#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "include/ordering/factors.h"
#include "include/ordering/gaussian.h"
#include "include/ordering/urgencies.h"
#include "include/parse/parse.h"
#include "include/engine-core/types.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for a Zobrist hash                                                                     */
/* ------------------------------------------------------------------------------------------------ */

typedef struct _move_zobrist_table_t {
    uint32_t piecefrom[NR_PIECES];
    uint32_t pieceto[NR_PIECES];
    uint32_t from[64];
    uint32_t to[64];
    uint32_t prompiece[5];
    uint32_t in_attack_range_after[2];
    uint32_t in_attack_range_before[2];
} move_zobrist_table_t;

extern move_zobrist_table_t move_zobrist_table;

/* initializes the Zobrist hash for moves; should only be called once */
void initialize_move_zobrist_table();
/* function that computes a unique move key */
int calculate_move_key(board_t* board, move_t* move);

/* ------------------------------------------------------------------------------------------------ */
/* functions for online training of a Bayesian move ranking model                                   */
/* ------------------------------------------------------------------------------------------------ */

/* this function should be called once and sets up the ranking update graph(s) */
void initialize_ranking_updates();
/* updates the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
void update(gaussian_t** urgencies_ptr, int no_hashes, double beta_squared);

/* ------------------------------------------------------------------------------------------------ */
/* functions for batch training of a Bayesian move ranking model                                    */
/* ------------------------------------------------------------------------------------------------ */

typedef struct _ranking_update_info_t {
    int no_moves;                   /* the number of moves in the ranking update */
    gaussian_mean_factor_info_t* g; /* the Gaussian mean factor to the latent urgencies */
    weighted_sum_factor_info_t* s;  /* the weighted sum factor between the latent urgencies and the diffs */
    greater_than_factor_info_t* h;  /* the greater-than factor for the pairwise ordering */

    gaussian_t* latent_urgency;               /* the latent urgencies */
    gaussian_t* diffs;                        /* the difference factors */
    gaussian_t* msg_from_g_to_latent_urgency; /* the messages from the Gaussian mean factor to latent urgencies */
    gaussian_t* msg_from_g_to_urgency;        /* the messages from the Gaussian mean factor to the actual urgencies */
    gaussian_t* msg_from_s_to_diffs;          /* the messages from the weighted sum factor to the sum/difference */
    gaussian_t* msg_from_s_to_top_urgency;    /* the messages from the weighted sum factor to the latent urgency of the move made */
    gaussian_t* msg_from_s_to_urgency;        /* the messages from the weighted sum factor to the latent urgency of the move not made */
    gaussian_t* msg_from_h_to_diffs;          /* the messages from the greater-than factor to the sum/difference */

    struct _ranking_update_info_t* next; /* pointer to the next ranking update info */
} ranking_update_info_t;

/* adds the factor graph that processes a single move made to the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
ranking_update_info_t* add_ranking_update_graph(ranking_update_info_t* root, gaussian_t** urgencies_ptr, int no_hashes, double beta_squared);
/* run message passing on the whole graph until convergence of epsilon; if base_filename is non-NULL then snapshots are stored after every iteration */
void refresh_update_graph(ranking_update_info_t* root, double epsilon, const char* base_filename);
/* deletes the linked list of ranking updates */
void delete_ranking_update_graphs(ranking_update_info_t* root);

/* ------------------------------------------------------------------------------------------------ */
/* functions and types for training from Chess game records                                         */
/* ------------------------------------------------------------------------------------------------ */

typedef struct _train_info_t {
    urgency_ht_entry_t* ht_urgencies; /* hash table of urgencies for each move (hash) */
    gaussian_t prior;                 /* prior for the urgencies */
    double beta;                      /* the standard deviation parameter for the latent urgencies */
    int full_training;                /* whether we are doing full training or incremental training */
    char* base_filename;              /* the base filename for storing snapshots of the model */
    int verbosity;                    /* the verbosity level */
} train_info_t;

/* trains a Bayesian ranking model from the replay of the games */
void train_model(chess_game_t** chess_games, int nr_of_games, train_info_t train_info);

/* ------------------------------------------------------------------------------------------------ */
/* functions for making predictions based on the Bayesian move ranking model                        */
/* ------------------------------------------------------------------------------------------------ */

/* computes the probability of all moves being the most urgent move */
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
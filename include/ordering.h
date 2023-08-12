#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "../include/gaussian.h"
#include "../include/factors.h"
#include "../include/types.h"

#define HT_GAUSSIAN_SIZE 9216000
#define MAX_MOVES 512

typedef struct _urgency_ht_list_entry_t {
    int move_key;                           /* true (non-clashing) value of the move */
    gaussian_t urgency;                     /* urgency of the move */
    struct _urgency_ht_list_entry_t* next;
} urgency_ht_entry_t;

/* hash table of urgencies for each move (hash) */
extern gaussian_t* ht_urgencies;

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the hash-table of a urgencies for the Bayesian move ranking model         */
/* ------------------------------------------------------------------------------------------------ */

/* initializes an urgency hashtable with standard Normals */
gaussian_t *initialize_ht_urgencies();
/* deletes the memory for a hashtable of urgencies */
void deletes_ht_urgencies(gaussian_t *ht);
/* hash function from move to urgencies hash-table index */
int calculate_order_hash(board_t* board, move_t* move);
/* loads a hash-table of urgencies from a file (only entries which are different from the prior) */
void load_ht_urgencies_from_binary_file(const char* file_name, gaussian_t *ht);
/* writes an urgencies hash-table to a file (only entries which are different from the prior) */
void write_ht_urgencies_to_binary_file(const char* file_name, const gaussian_t *ht);

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
    int no_moves;                               /* the number of moves in the ranking update */
    gaussian_mean_factor_info_t* g;             /* the Gaussian mean factor to the latent urgencies */
    weighted_sum_factor_info_t* s;              /* the weighted sum factor between the latent urgencies and the diffs */
    greater_than_factor_info_t* h;              /* the greater-than factor for the pairwise ordering */

    gaussian_t* latent_urgency;                 /* the latent urgencies */
    gaussian_t* diffs;                          /* the difference factors */
    gaussian_t* msg_from_g_to_latent_urgency;   /* the messages from the Gaussian mean factor to latent urgencies */
    gaussian_t* msg_from_g_to_urgency;          /* the messages from the Gaussian mean factor to the actual urgencies */
    gaussian_t* msg_from_s_to_diffs;            /* the messages from the weighted sum factor to the sum/difference */
    gaussian_t* msg_from_s_to_top_urgency;      /* the messages from the weighted sum factor to the latent urgency of the move made */
    gaussian_t* msg_from_s_to_urgency;          /* the messages from the weighted sum factor to the latent urgency of the move not made */
    gaussian_t* msg_from_h_to_diffs;            /* the messages from the greater-than factor to the sum/difference */

    struct _ranking_update_info_t* next;        /* pointer to the next ranking update info */
} ranking_update_info_t;

/* adds the factor graph that processes a single move made to the urgency belief distributions indexed by the no_hashes many move hashes given in hashes */
ranking_update_info_t* add_ranking_update_graph(ranking_update_info_t* root, gaussian_t** urgencies_ptr, int no_hashes, double beta_squared);
/* run message passing on the whole graph until convergence of epsilon; if base_filename is non-NULL then snapshots are stored after every iteration */
void refresh_update_graph(ranking_update_info_t *root, double epsilon, const char* base_filename);
/* deletes the linked list of ranking updates */
void delete_ranking_update_graphs(ranking_update_info_t* root);


/* ------------------------------------------------------------------------------------------------ */
/* functions for making predictions based on the Bayesian move ranking model                        */
/* ------------------------------------------------------------------------------------------------ */

/* computes the probability of all moves being the most urgent move */
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
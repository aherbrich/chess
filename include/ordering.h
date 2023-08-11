#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "../include/gaussian.h"
#include "../include/types.h"

#define HT_GAUSSIAN_SIZE 4608000
#define MAX_MOVES 512

/* table of gaussians corresponding to moves */
extern gaussian_t* ht_gaussians;

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the hash-table of a Bayesian move ranking model                           */
/* ------------------------------------------------------------------------------------------------ */

/* initializes a Gaussian hashtable with standard Normals */
gaussian_t *initialize_ht_gaussians();
/* deletes the memory for a hashtable of Gaussians */
void deletes_ht_gaussians(gaussian_t *ht);
/* hash function from move to gaussian ht index */
int calculate_order_hash(board_t* board, move_t* move);
/* loads a Gaussian hash-table from a file (only entries which are different from the prior) */
void load_ht_gaussians_from_binary_file(const char* file_name, gaussian_t *ht);
/* writes a Gaussian hash-table to a file (only entries which are different from the prior) */
void write_ht_gaussians_to_binary_file(const char* file_name, const gaussian_t *ht);

/* ------------------------------------------------------------------------------------------------ */
/* functions for online training of a Bayesian move ranking model                                   */
/* ------------------------------------------------------------------------------------------------ */

/* this function should be called once and sets up the ranking update graph(s) */
void initalize_ranking_updates();
/* updates the urgency belief distributions indexed by the no_hashes given */
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared);

/* ------------------------------------------------------------------------------------------------ */
/* functions for making predcitions based on the Bayesian move ranking model                        */
/* ------------------------------------------------------------------------------------------------ */

/* computes the probability of all moves being the most urgent moves */
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
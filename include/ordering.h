#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "../include/types.h"
#include "../include/gaussian.h"

#define HTSIZEGAUSSIAN 4608000
#define MAX_MOVES 512


/* table of gaussians corresponding to moves */
extern gaussian_t* ht_gaussians;

gaussian_t* initialize_ht_gaussians(double mean, double var);
int calculate_order_hash(board_t* board, move_t* move);
gaussian_t* load_ht_gaussians_by_file();

void initalize_ranking_updates();
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared);
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "../include/gaussian.h"
#include "../include/types.h"

#define HT_GAUSSIAN_SIZE 4608000
#define MAX_MOVES 512

/* table of gaussians corresponding to moves */
extern gaussian_t* ht_gaussians;

gaussian_t *initialize_ht_gaussians();
void deletes_ht_gaussians(gaussian_t *ht);
int calculate_order_hash(board_t* board, move_t* move);
void load_ht_gaussians_from_binary_file(const char* file_name, gaussian_t *ht);
void write_ht_gaussians_to_binary_file(const char* file_name, const gaussian_t *ht);

void initalize_ranking_updates();
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared);
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
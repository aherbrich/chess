#ifndef __ORDERING_H__
#define __ORDERING_H__

#include "../include/gaussian.h"
#include "../include/types.h"

#define HT_GAUSSIAN_SIZE 4608000
#define MAX_MOVES 512

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
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared);

/* ------------------------------------------------------------------------------------------------ */
/* functions for making predictions based on the Bayesian move ranking model                        */
/* ------------------------------------------------------------------------------------------------ */

/* computes the probability of all moves being the most urgent move */
void predict_move_probabilities(gaussian_t* urgency_beliefs, double* prob, int* hashes, int no_hashes, double beta_squared);

#endif
#ifndef __RANKING_H__
#define __RANKING_H__

#include "gaussian.h"

/* this function should be called once and sets up the ranking update graph(s) */
void initalize_ranking_updates();

/* updates the urgency belief distributions indexed by the no_hashes given */
void update(gaussian_t* urgency_beliefs, int* hashes, int no_hashes, double beta_squared);

#endif
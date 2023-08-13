#ifndef __FEATURES_H__
#define __FEATURES_H__

#include "include/engine-core/types.h"
#include "include/train-eval/linalg.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for training of a linear regression evaluation model                                   */
/* ------------------------------------------------------------------------------------------------ */

/* calculates feature matrix */
void calculate_feautures(board_t* board, matrix_t* X, int idx);

#endif
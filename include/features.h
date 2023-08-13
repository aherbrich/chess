#ifndef __FEATURES_H__
#define __FEATURES_H__

#include "../include/types.h"
#include "../include/linalg.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for training of a linear regression evaluation model                                   */
/* ------------------------------------------------------------------------------------------------ */

/* calculates feature matrix */
void calculate_feautures(board_t* board, matrix_t* X, int idx);

#endif
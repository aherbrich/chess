#ifndef __ORDERING_H__
#define __ORDERING_H__

#include <string.h>
#include <stdio.h>

#include "../include/chess.h"
#include "../include/gaussian.h"

#define HTSIZEGAUSSIAN 4608000

/* struct which holds move ranking information */
typedef struct _move_ranking_info_t{
    piece_t piece_moved;
    piece_t piece_captured;
    piece_t piece_prom;
    square_t from;
    square_t to;
} move_ranking_info_t;

gaussian_t* ht_gaussians;

gaussian_t* initialize_ht_gaussians(double mean, double var);

int move_order_hash_by_infostruct(move_ranking_info_t* info);
int move_order_hash(piece_t piece_moved, square_t from, piece_t piece_captured, square_t to, piece_t piece_prom);

#endif
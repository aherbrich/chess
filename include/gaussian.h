#include "math.h"
#include "stdio.h"

#ifndef __GAUSSIAN_H__
#define __GAUSSIAN_H__
typedef struct _gaussian1D_t {
    double tau;
    double rho;
} gaussian1D_t;

gaussian1D_t init_gaussian1D(double tau, double rho);
gaussian1D_t init_gaussian1D_standard_normal();
gaussian1D_t init_gaussian1D_from_mean_and_variance(double mean, double var);

double mean(gaussian1D_t gaussian1D);
double variance(gaussian1D_t gaussian1D);
double absdiff(gaussian1D_t g1, gaussian1D_t g2);

gaussian1D_t gaussian1D_mult(gaussian1D_t g1, gaussian1D_t g2);
gaussian1D_t gaussian1D_div(gaussian1D_t g1, gaussian1D_t g2);

double log_norm_product(gaussian1D_t g1, gaussian1D_t g2);
double log_norm_ratio(gaussian1D_t g1, gaussian1D_t g2);

void print_gaussian1D(gaussian1D_t gaussian1D);

#endif
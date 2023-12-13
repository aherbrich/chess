#ifndef __GAUSSIAN_H__
#define __GAUSSIAN_H__

typedef struct _gaussian_t {
    double tau;
    double rho;
} gaussian_t;

gaussian_t init_gaussian1D(double tau, double rho);
gaussian_t init_gaussian1D_standard_normal(void);
gaussian_t init_gaussian1D_from_mean_and_variance(double mean, double var);

double mean(gaussian_t gaussian1D);
double variance(gaussian_t gaussian1D);
double abs_diff(gaussian_t g1, gaussian_t g2);

gaussian_t gaussian1D_mult(gaussian_t g1, gaussian_t g2);
gaussian_t gaussian1D_div(gaussian_t g1, gaussian_t g2);

double log_norm_product(gaussian_t g1, gaussian_t g2);
double log_norm_ratio(gaussian_t g1, gaussian_t g2);

void print_gaussian1D(gaussian_t gaussian1D);

#endif

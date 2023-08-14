#include <stdio.h>
#include <math.h>

#include "include/ordering/gaussian.h"

/* initializes a 1D Gaussian in natural parameters */
gaussian_t init_gaussian1D(double tau, double rho) {
    gaussian_t gaussian1D;
    gaussian1D.tau = tau;
    gaussian1D.rho = rho;
    return gaussian1D;
}

/* initializes a standard Normal */
gaussian_t init_gaussian1D_standard_normal() {
    return init_gaussian1D(0, 1);
}

/* initializes a 1D Gaussian in mean & variance parameters */
gaussian_t init_gaussian1D_from_mean_and_variance(double mean, double var) {
    gaussian_t gaussian1D;
    gaussian1D.tau = mean / var;
    gaussian1D.rho = 1.0 / var;
    return gaussian1D;
}

/* returns the mean of the 1D Gaussian */
double mean(gaussian_t gaussian1D) {
    return gaussian1D.tau / gaussian1D.rho;
}

/* returns the variance of the 1D Gaussian */
double variance(gaussian_t gaussian1D) {
    return 1.0 / gaussian1D.rho;
}

/* returns the absolute difference between two 1D Gaussian */
double abs_diff(gaussian_t g1, gaussian_t g2) {
    return fmax(fabs(g1.tau - g2.tau), sqrt(fabs(g1.rho - g2.rho)));
}

/* returns the product of two 1D Gaussian */
gaussian_t gaussian1D_mult(gaussian_t g1, gaussian_t g2) {
    gaussian_t gaussian1D;
    gaussian1D.tau = g1.tau + g2.tau;
    gaussian1D.rho = g1.rho + g2.rho;
    return gaussian1D;
}

/* returns the ratio of two 1D Gaussian */
gaussian_t gaussian1D_div(gaussian_t g1, gaussian_t g2) {
    gaussian_t gaussian1D;
    gaussian1D.tau = g1.tau - g2.tau;
    gaussian1D.rho = g1.rho - g2.rho;
    return gaussian1D;
}

/* returns the log-normalization constant of the product of two 1D Gaussian */
double log_norm_product(gaussian_t g1, gaussian_t g2) {
    if (g1.rho == 0.0 || g2.rho == 0.0) {
        return 0.0;
    } else {
        double var_sum = variance(g1) + variance(g2);
        double mean_diff = mean(g1) - mean(g2);
        return (-0.5 * (log(2 * M_PI * var_sum) + mean_diff * mean_diff / var_sum));
    }
}

/* returns the log-normalization constant of the ratio of two 1D Gaussian */
double log_norm_ratio(gaussian_t g1, gaussian_t g2) {
    if (g1.rho == 0.0 || g2.rho == 0.0) {
        return 0.0;
    } else {
        double var_g2 = variance(g2);
        double var_diff = var_g2 - variance(g1);
        if (var_diff == 0.0) {
            return 0.0;
        } else {
            double mean_diff = mean(g1) - mean(g2);
            return (log(var_g2) + 0.5 * (log(2 * M_PI / var_diff) + mean_diff * mean_diff / var_diff));
        }
    }
}

/* prints the mean and variance of a 1D Gaussian */
void print_gaussian1D(gaussian_t gaussian1D) {
    if (gaussian1D.rho == 0.0) {
        printf("μ = 0, σ = Inf\n");
    } else {
        printf("μ = %.3f σ = %.3f\n", mean(gaussian1D), sqrt(variance(gaussian1D)));
    }
}

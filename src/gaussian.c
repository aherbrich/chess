#include "../include/gaussian.h"

gaussian1D_t init_gaussian1D(double tau, double rho){
    gaussian1D_t gaussian1D;
    gaussian1D.tau = tau;
    gaussian1D.rho = rho;
    return gaussian1D;
}

gaussian1D_t init_gaussian1D_standard_normal(){
    return init_gaussian1D(0,1);
}

gaussian1D_t init_gaussian1D_from_mean_and_variance(double mean, double var){
    gaussian1D_t gaussian1D;
    gaussian1D.tau = mean/var;
    gaussian1D.rho = 1.0/var;
    return gaussian1D;
}

double mean(gaussian1D_t gaussian1D){
    return gaussian1D.tau / gaussian1D.rho;
} 

double variance(gaussian1D_t gaussian1D){
    return 1.0 / gaussian1D.rho;
}

double absdiff(gaussian1D_t g1, gaussian1D_t g2){
    return fmax(fabs(g1.tau - g2.tau), sqrt(fabs(g1.rho - g2.rho)));
}

gaussian1D_t gaussian1D_mult(gaussian1D_t g1, gaussian1D_t g2){
    gaussian1D_t gaussian1D;
    gaussian1D.tau = g1.tau + g2.tau;
    gaussian1D.rho = g1.rho + g2.rho;
    return gaussian1D;
}

gaussian1D_t gaussian1D_div(gaussian1D_t g1, gaussian1D_t g2){
    gaussian1D_t gaussian1D;
    gaussian1D.tau = g1.tau - g2.tau;
    gaussian1D.rho = g1.rho - g2.rho;
    return gaussian1D;
}

double log_norm_product(gaussian1D_t g1, gaussian1D_t g2){
    if(g1.rho == 0.0 || g2.rho == 0.0){
        return 0.0;
    } else{
        double var_sum = variance(g1) + variance(g2);
        printf("%f\n", var_sum);
        double mean_diff = mean(g1) - mean(g2);
        printf("%f\n", mean_diff);
        return (-0.5 * (log(2 * M_PI * var_sum) + mean_diff * mean_diff / var_sum));
    }
}

double log_norm_ratio(gaussian1D_t g1, gaussian1D_t g2){
    if(g1.rho == 0.0 || g2.rho == 0.0){
        return 0.0;
    } else{
        double var_g2 = variance(g2);
        double var_diff = var_g2 - variance(g1);
        if(var_diff == 0.0){
            return 0.0;
        } else{
            double mean_diff = mean(g1) - mean(g2);
            return (log(var_g2) + 0.5 * (log(2 * M_PI / var_diff) + mean_diff * mean_diff / var_diff));
        }
    }
}

void print_gaussian1D(gaussian1D_t gaussian1D){
    if (gaussian1D.rho == 0.0){
        printf("μ = 0, σ = Inf\n");
    } else {
        printf("μ = %.3f σ = %.3f\n", mean(gaussian1D), sqrt(variance(gaussian1D)));
    }
}


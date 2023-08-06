#include <math.h>

#include "../include/gaussian.h"
#include "../include/factors.h"

#include <math.h>
#include <stdio.h>

/* update function for the Gaussian factor */
double gaussian_factor_update(gaussian_factor_info_t* info) {
    gaussian_t old_marginal = *info->marginal;
    *info->marginal = gaussian1D_mult(gaussian1D_div(old_marginal, *info->msg), info->g);
    *info->msg = info->g;
    return absdiff(*info->marginal, old_marginal);
}

/* log of variable normalization of Gaussian factor */
double gaussian_factor_log_variable_norm(gaussian_factor_info_t* info) {
    double logZ = log_norm_product(*info->marginal, *info->msg);
    *info->marginal = gaussian1D_mult(*info->marginal, *info->msg);
    return logZ;
}

/* log of factor normalization of Gaussian factor */
double gaussian_factor_log_factor_norm(gaussian_factor_info_t* info) {
    return 0.0;
}

/* update function for the Gaussian mean factor to the variable */
double gaussian_mean_factor_update_to_variable(gaussian_mean_factor_info_t* info) {
    gaussian_t msg_back = gaussian1D_div(*info->mean_marginal, *info->mean_msg);
    double c = 1 / (1 + info->beta_squared * msg_back.rho);
    gaussian_t new_msg = init_gaussian1D(c * msg_back.tau, c * msg_back.rho);

    gaussian_t old_var_marginal = *info->var_marginal;
    *info->var_marginal = gaussian1D_mult(gaussian1D_div(old_var_marginal, *info->var_msg), new_msg);
    *info->var_msg = new_msg;
    return absdiff(*info->var_marginal, old_var_marginal);
}

/* update function for the Gaussian mean factor to the mean */
double gaussian_mean_factor_update_to_mean(gaussian_mean_factor_info_t* info) {
    gaussian_t msg_back = gaussian1D_div(*info->var_marginal, *info->var_msg);
    double c = 1 / (1 + info->beta_squared * msg_back.rho);
    gaussian_t new_msg = init_gaussian1D(c * msg_back.tau, c * msg_back.rho);

    gaussian_t old_mean_marginal = *info->mean_marginal;
    *info->mean_marginal = gaussian1D_mult(gaussian1D_div(old_mean_marginal, *info->mean_msg), new_msg);
    *info->mean_msg = new_msg;
    return absdiff(*info->mean_marginal, old_mean_marginal);
}

/* log of variable normalization of Gaussian mean factor */
double gaussian_mean_factor_log_variable_norm(gaussian_mean_factor_info_t* info) {
    double logZ = log_norm_product(*info->var_marginal, *info->var_msg);
    logZ += log_norm_product(*info->mean_marginal, *info->mean_msg);
    *info->var_marginal = gaussian1D_mult(*info->var_marginal, *info->var_msg);
    *info->mean_marginal = gaussian1D_mult(*info->mean_marginal, *info->mean_msg);
    return logZ;
}

/* log of factor normalization of Gaussian mean factor */
double gaussian_mean_factor_log_factor_norm(gaussian_mean_factor_info_t* info) {
    return (log_norm_ratio(*info->mean_marginal, *info->mean_msg));
}

/* update function for the weighted sum factor to the sum variable */
double weighted_sum_factor_update_to_sum(weighted_sum_factor_info_t* info) {
    gaussian_t msg_back_s1 = gaussian1D_div(*info->s1_marginal, *info->s1_msg);
    gaussian_t msg_back_s2 = gaussian1D_div(*info->s2_marginal, *info->s2_msg);
    gaussian_t new_msg = init_gaussian1D_from_mean_and_variance(
        info->a1 * mean(msg_back_s1) + info->a2 * mean(msg_back_s2),
        info->a1 * info->a1 * variance(msg_back_s1) + info->a2 * info->a2 * variance(msg_back_s2));
    gaussian_t old_sum_marginal = *info->sum_marginal;
    *info->sum_marginal = gaussian1D_mult(gaussian1D_div(old_sum_marginal, *info->sum_msg), new_msg);
    *info->sum_msg = new_msg;
    return absdiff(*info->sum_marginal, old_sum_marginal);
}

/* update function for the weighted sum factor to the summand 1 variable */
double weighted_sum_factor_update_to_summand1(weighted_sum_factor_info_t* info) {
    gaussian_t msg_back_s2 = gaussian1D_div(*info->s2_marginal, *info->s2_msg);
    gaussian_t msg_back_sum = gaussian1D_div(*info->sum_marginal, *info->sum_msg);
    gaussian_t new_msg = init_gaussian1D_from_mean_and_variance(
        (mean(msg_back_sum) - info->a2 * mean(msg_back_s2)) / info->a1,
        (variance(msg_back_sum) + info->a2 * info->a2 * variance(msg_back_s2)) / (info->a1 * info->a1));
    gaussian_t old_s1_marginal = *info->s1_marginal;
    *info->s1_marginal = gaussian1D_mult(gaussian1D_div(old_s1_marginal, *info->s1_msg), new_msg);
    *info->s1_msg = new_msg;
    return absdiff(*info->s1_marginal, old_s1_marginal);
}

/* update function for the weighted sum factor to the summand 2 variable */
double weighted_sum_factor_update_to_summand2(weighted_sum_factor_info_t* info) {
    gaussian_t msg_back_s1 = gaussian1D_div(*info->s1_marginal, *info->s1_msg);
    gaussian_t msg_back_sum = gaussian1D_div(*info->sum_marginal, *info->sum_msg);
    gaussian_t new_msg = init_gaussian1D_from_mean_and_variance(
        (mean(msg_back_sum) - info->a1 * mean(msg_back_s1)) / info->a2,
        (variance(msg_back_sum) + info->a1 * info->a1 * variance(msg_back_s1)) / (info->a2 * info->a2));
    gaussian_t old_s2_marginal = *info->s2_marginal;
    *info->s2_marginal = gaussian1D_mult(gaussian1D_div(old_s2_marginal, *info->s2_msg), new_msg);
    *info->s2_msg = new_msg;
    return absdiff(*info->s2_marginal, old_s2_marginal);
}

/* log of variable normalization of weighted sum factor */
double weighted_sum_factor_log_variable_norm(weighted_sum_factor_info_t* info) {
    double logZ = log_norm_product(*info->s1_marginal, *info->s1_msg);
    logZ += log_norm_product(*info->s2_marginal, *info->s2_msg);
    logZ += log_norm_product(*info->sum_marginal, *info->sum_msg);
    *info->s1_marginal = gaussian1D_mult(*info->s1_marginal, *info->s1_msg);
    *info->s2_marginal = gaussian1D_mult(*info->s2_marginal, *info->s2_msg);
    *info->sum_marginal = gaussian1D_mult(*info->sum_marginal, *info->sum_msg);
    return logZ;
}

/* log of factor normalization of weighted sum factor */
double weighted_sum_factor_log_factor_norm(weighted_sum_factor_info_t* info) {
    return (log_norm_ratio(*info->s1_marginal, *info->s1_msg) + log_norm_ratio(*info->s2_marginal, *info->s2_msg));
}

/* update function for the Greater than factor */
double greater_than_factor_update(greater_than_factor_info_t* info) {
    gaussian_t msg_back = gaussian1D_div(*info->marginal, *info->msg);
    double a = msg_back.tau / sqrt(msg_back.rho);
    double b = info->epsilon * sqrt(msg_back.rho);
    double x = a - b;
    double v = 0.3989422804014327 * exp(-x * x / 2) / (0.5 * erfc(-x * M_SQRT1_2));
    double w = v * (v + x);
    double c = 1.0 - w;
    gaussian_t old_marginal = *info->marginal;
    *info->marginal = init_gaussian1D((msg_back.tau + sqrt(msg_back.rho) * v) / c, msg_back.rho / c);
    *info->msg = gaussian1D_div(*info->marginal, msg_back);
    return absdiff(*info->marginal, old_marginal);
}

/* log of variable normalization of greater-than factor */
double greater_than_factor_log_variable_norm(greater_than_factor_info_t* info) {
    double logZ = log_norm_product(*info->marginal, *info->msg);
    *info->marginal = gaussian1D_mult(*info->marginal, *info->msg);
    return logZ;
}

/* log of factor normalization of greater-than factor */
double greater_than_factor_log_factor_norm(greater_than_factor_info_t* info) {
    gaussian_t msg_back = gaussian1D_div(*info->marginal, *info->msg);
    return (-log_norm_product(msg_back, *info->msg) +
            log((0.5 * erfc(-((mean(msg_back) - info->epsilon) / sqrt(variance(msg_back))) * M_SQRT1_2))));
}

#ifndef __FACTORS_H__
#define __FACTORS_H__

#include "gaussian.h"

typedef struct _gaussian_factor_info_t {
    gaussian_t g;
    gaussian_t* marginal;
    gaussian_t* msg;
} gaussian_factor_info_t;
double gaussian_factor_update(gaussian_factor_info_t* info);
double gaussian_factor_log_variable_norm(gaussian_factor_info_t* info);
double gaussian_factor_log_factor_norm(gaussian_factor_info_t* info);


typedef struct _gaussian_mean_factor_info_t {
    double beta_squared;
    gaussian_t* var_marginal;
    gaussian_t* mean_marginal;
    gaussian_t* var_msg;
    gaussian_t* mean_msg;
} gaussian_mean_factor_info_t;
double gaussian_mean_factor_update_to_variable(gaussian_mean_factor_info_t* info);
double gaussian_mean_factor_update_to_mean(gaussian_mean_factor_info_t* info);
double gaussian_mean_factor_log_variable_norm(gaussian_mean_factor_info_t* info);
double gaussian_mean_factor_log_factor_norm(gaussian_mean_factor_info_t* info);


typedef struct _weighted_sum_factor_info_t {
    double a1;
    double a2;
    gaussian_t* s1_marginal;
    gaussian_t* s2_marginal;
    gaussian_t* sum_marginal;
    gaussian_t* s1_msg;
    gaussian_t* s2_msg;
    gaussian_t* sum_msg;
} weighted_sum_factor_info_t;
double weighted_sum_factor_update_to_sum(weighted_sum_factor_info_t* info);
double weighted_sum_factor_update_to_summand1(weighted_sum_factor_info_t* info);
double weighted_sum_factor_update_to_summand2(weighted_sum_factor_info_t* info);
double weighted_sum_factor_log_variable_norm(weighted_sum_factor_info_t* info);
double weighted_sum_factor_log_factor_norm(weighted_sum_factor_info_t* info);


typedef struct _greater_than_factor_info_t {
    double epsilon;
    gaussian_t* marginal;
    gaussian_t* msg;
} greater_than_factor_info_t;
double greater_than_factor_update(greater_than_factor_info_t* info);
double greater_than_factor_log_variable_norm(greater_than_factor_info_t* info);
double greater_than_factor_log_factor_norm(greater_than_factor_info_t* info);

#endif
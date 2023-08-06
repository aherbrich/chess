#include "stdio.h"
#include "../include/gaussian.h"


typedef struct _factor_t{
    double (*update)(int);
    double (*log_var_norm)();
    double (*log_factor_norm)();
} factor_t;


factor_t gaussian_factor(gaussian1D_t g, gaussian1D_t* g_other){
    gaussian1D_t msg = init_gaussian1D(0.0, 0.0);


    double update(int i){
        if(i == 1){
            gaussian1D_t old_marginal = (*g_other);
            gaussian1D_t new_marginal = gaussian1D_mult(gaussian1D_div(old_marginal, msg), g);
            *g_other = new_marginal;
        }
    }

};
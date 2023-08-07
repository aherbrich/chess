#include <stdio.h>
#include <math.h>

#include "../include/gaussian.h"
#include "../include/factors.h"
#include "../include/prettyprint.h"

uint64_t HISTORY_HASHES[MAXPLIES];

/* runs the Gaussian tests */
int gaussian_tests() {
    int fail_counter = 0;

    printf("Running Gaussian tests...\n");
    
    if (mean(init_gaussian1D(1, 2)) != 0.5) {
        printf("%sFAIL%s: mean(init_gaussian1D(1, 2)) != 0.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(init_gaussian1D_from_mean_and_variance(1, 2)) != 1.0) {
        printf("%sFAIL%s: mean(init_gaussian1D_from_mean_and_variance(1, 2)) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(init_gaussian1D(1, 2)) != 0.5) {
        printf("%sFAIL%s: variance(init_gaussian1D(1, 2)) != 0.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(init_gaussian1D_from_mean_and_variance(1, 2)) != 2.0) {
        printf("%sFAIL%s: variance(init_gaussian1D_from_mean_and_variance(1, 2)) != 2.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (absdiff(init_gaussian1D(0, 1), init_gaussian1D(0, 2)) != 1.0) {
        printf("%sFAIL%s: absdiff(init_gaussian1D(0, 1), init_gaussian1D(0, 2)) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (absdiff(init_gaussian1D(0, 1), init_gaussian1D(0, 3)) != 1.4142135623730951) {
        printf("%sFAIL%s: absdiff(init_gaussian1D(0, 1), init_gaussian1D(0, 3)) != 1.4142135623730951\n", Color_RED, Color_END);
        fail_counter++;
    }

    gaussian_t g1 = init_gaussian1D_standard_normal();
    if (absdiff(gaussian1D_mult(g1,g1), init_gaussian1D(0, 2)) != 0.0) {
        printf("%sFAIL%s: absdiff(gaussian1D_mult(g1,g1), init_gaussian1D(0, 2)) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    gaussian_t g2 = init_gaussian1D(0,0.5);
    if (absdiff(gaussian1D_div(g1,g2), init_gaussian1D(0, 0.5)) != 0.0) {
        printf("%sFAIL%s: absdiff(gaussian1D_mult(g1,g2), init_gaussian1D(0, 0.5)) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (log_norm_product(g1,g1) != -1.2655121234846454) {
        printf("%sFAIL%s: log_norm_product(g1,g1) != -1.2655121234846454\n", Color_RED, Color_END);
        fail_counter++;        
    }

    if (log_norm_ratio(g1,g2) != 1.612085713764618) {
        printf("%sFAIL%s: log_norm_ratio(g1,g2) != 1.612085713764618\n", Color_RED, Color_END);
        fail_counter++;        
    }

    return fail_counter;
}

/* runs the Gaussian factor tests */
int gaussian_factor_tests() {
    int fail_counter = 0;

    printf("Running Gaussian factor tests...\n");

    gaussian_t s = init_gaussian1D(0, 0);
    gaussian_t msg1_to_s = init_gaussian1D(0, 0);
    gaussian_t msg2_to_s = init_gaussian1D(0, 0);

    gaussian_factor_info_t factor = { init_gaussian1D_from_mean_and_variance(2,42), &s, &msg1_to_s };
    gaussian_factor_info_t factor2 = { init_gaussian1D_from_mean_and_variance(1,1), &s, &msg2_to_s };

    if (gaussian_factor_update(&factor) != 0.1543033499620919) {
        printf("%sFAIL%s: gaussian_factor_update(&factor) != 0.1543033499620919\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_update(&factor) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_update(&factor) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_update(&factor2) != 1.0) {
        printf("%sFAIL%s: gaussian_factor_update(&factor2) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s) != 1.0232558139534884) {
        printf("%sFAIL%s: mean(s) != 1.0232558139534884\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s) != 0.9767441860465117) {
        printf("%sFAIL%s: variance(s) != 0.9767441860465117\n", Color_RED, Color_END);
        fail_counter++;
    }

    s = init_gaussian1D(0, 0);

    if (gaussian_factor_log_variable_norm(&factor) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&factor) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_log_variable_norm(&factor2) != -2.8111664980281983) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&factor2) != -2.8111664980281983\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s) != 1.0232558139534884) {
        printf("%sFAIL%s: mean(s) != 1.0232558139534884\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s) != 0.9767441860465117) {
        printf("%sFAIL%s: variance(s) != 0.9767441860465117\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_log_factor_norm(&factor) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_factor_norm(&factor) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    return fail_counter;
}

/* runs the Gaussian mean factor tests */
int gaussian_mean_factor_tests() {
    int fail_counter = 0;

    printf("Running Gaussian mean factor tests...\n");

    gaussian_t s1 = init_gaussian1D(0, 0);
    gaussian_t s2 = init_gaussian1D(0, 0);
    gaussian_t msg_f_to_s1 = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s1 = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s2 = init_gaussian1D(0, 0);

    gaussian_factor_info_t f = { init_gaussian1D_from_mean_and_variance(3,1), &s1, &msg_f_to_s1 };
    gaussian_mean_factor_info_t g = { 0.5, &s2, &s1, &msg_g_to_s2, &msg_g_to_s1 };

    if (gaussian_factor_update(&f) != 3.0) {
        printf("%sFAIL%s: gaussian_factor_update(&f) != 3.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_mean_factor_update_to_variable(&g) != 2.0) {
        printf("%sFAIL%s: gaussian_mean_factor_update_to_variable(&g) != 2.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s1) != 3.0) {
        printf("%sFAIL%s: mean(s1) != 3.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s2) != 3.0) {
        printf("%sFAIL%s: mean(s2) != 3.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s1) != 1.0) {
        printf("%sFAIL%s: variance(s1) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s2) != 1.5) {
        printf("%sFAIL%s: variance(s2) != 1.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_mean_factor_update_to_mean(&g) != 0.0) {
        printf("%sFAIL%s: gaussian_mean_factor_update_to_mean(&g) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    s1 = init_gaussian1D(0, 0);
    s2 = init_gaussian1D(0, 0);

    if (gaussian_factor_log_variable_norm(&f) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&f) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_mean_factor_log_variable_norm(&g) != 0.0) {
        printf("%sFAIL%s: gaussian_mean_factor_log_variable_norm(&g) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s1) != 3.0) {
        printf("%sFAIL%s: mean(s1) != 3.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (mean(s2) != 3.0) {
        printf("%sFAIL%s: mean(s2) != 3.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s1) != 1.0) {
        printf("%sFAIL%s: variance(s1) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (variance(s2) != 1.5) {
        printf("%sFAIL%s: variance(s2) != 1.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_mean_factor_log_factor_norm(&g) != 0.0) {
        printf("%sFAIL%s: gaussian_mean_factor_log_factor_norm(&g) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    return fail_counter;
}

/* runs the weighted sum factor tests */
int weighted_sum_factor_tests() {
    int fail_counter = 0;

    printf("Running weighted sum factor tests...\n");

    gaussian_t s1 = init_gaussian1D(0, 0);
    gaussian_t s2 = init_gaussian1D(0, 0);
    gaussian_t s3 = init_gaussian1D(0, 0);
    gaussian_t msg_f1_to_s1 = init_gaussian1D(0, 0);
    gaussian_t msg_f2_to_s2 = init_gaussian1D(0, 0);
    gaussian_t msg_f3_to_s3 = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s1 = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s2 = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s3 = init_gaussian1D(0, 0);

    gaussian_factor_info_t f1 = { init_gaussian1D_from_mean_and_variance(1,1), &s1, &msg_f1_to_s1 };
    gaussian_factor_info_t f2 = { init_gaussian1D_from_mean_and_variance(2,4), &s2, &msg_f2_to_s2 };
    gaussian_factor_info_t f3 = { init_gaussian1D_from_mean_and_variance(2,0.5), &s3, &msg_f3_to_s3 };
    weighted_sum_factor_info_t g = { 0.5, 0.5, &s1, &s2, &s3, &msg_g_to_s1, &msg_g_to_s2, &msg_g_to_s3 };


    if (gaussian_factor_update(&f1) != 1.0) {
        printf("%sFAIL%s: gaussian_factor_update(&f1) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_update(&f2) != 0.5) {
        printf("%sFAIL%s: gaussian_factor_update(&f2) != 0.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (weighted_sum_factor_update_to_sum(&g) != 1.2) {
        printf("%sFAIL%s: weighted_sum_factor_update_to_sum(&g) != 1.2\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s3)- 1.5) > 1e-6) {
        printf("%sFAIL%s: mean(s3) != 1.5\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s3) - 1.25) > 1e-6) {
        printf("%sFAIL%s: variance(s3) != 1.25\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_update(&f3) != 4.0) {
        printf("%sFAIL%s: gaussian_factor_update(&f3) != 4.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s3)- 1.8571428571428574) > 1e-6) {
        printf("%sFAIL%s: mean(s3) != 1.8571428571428574\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s3) - 0.35714285714285715) > 1e-6) {
        printf("%sFAIL%s: variance(s3) != 0.35714285714285715\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(weighted_sum_factor_update_to_summand1(&g) - 0.40824829046386313) > 1e-6) {
        printf("%sFAIL%s: weighted_sum_factor_update_to_summand1(&g) != 0.40824829046386313\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s1)- 1.142857142857143) > 1e-6) {
        printf("%sFAIL%s: mean(s1) != 1.142857142857143\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s1) - 0.8571428571428571) > 1e-6) {
        printf("%sFAIL%s: variance(s1) != 0.8571428571428571\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(weighted_sum_factor_update_to_summand2(&g) - 1.0) > 1e-6) {
        printf("%sFAIL%s: weighted_sum_factor_update_to_summand2(&g) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s2)- 2.571428571428572) > 1e-6) {
        printf("%sFAIL%s: mean(s2) != 2.571428571428572\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s2) - 1.7142857142857144) > 1e-6) {
        printf("%sFAIL%s: variance(s2) != 1.7142857142857144\n", Color_RED, Color_END);
        fail_counter++;
    }

    s1 = init_gaussian1D(0, 0);
    s2 = init_gaussian1D(0, 0);
    s3 = init_gaussian1D(0, 0);

    if (gaussian_factor_log_variable_norm(&f1) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&f1) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_log_variable_norm(&f2) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&f2) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (gaussian_factor_log_variable_norm(&f3) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&f3) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (weighted_sum_factor_log_variable_norm(&g) != -5.196819356922758) {
        printf("%sFAIL%s: weighted_sum_factor_log_variable_norm(&g) != -5.196819356922758\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s1)- 1.142857142857143) > 1e-6) {
        printf("%sFAIL%s: mean(s1) != 1.142857142857143\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s1) - 0.8571428571428571) > 1e-6) {
        printf("%sFAIL%s: variance(s1) != 0.8571428571428571\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s2)- 2.571428571428572) > 1e-6) {
        printf("%sFAIL%s: mean(s2) != 2.571428571428572\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s2) - 1.7142857142857144) > 1e-6) {
        printf("%sFAIL%s: variance(s2) != 1.7142857142857144\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s3)- 1.8571428571428574) > 1e-6) {
        printf("%sFAIL%s: mean(s3) != 1.8571428571428574\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s3) - 0.35714285714285715) > 1e-6) {
        printf("%sFAIL%s: variance(s3) != 0.35714285714285715\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (weighted_sum_factor_log_factor_norm(&g) != 3.926644358321802) {
        printf("%sFAIL%s: weighted_sum_factor_log_factor_norm(&g) != 3.926644358321802\n", Color_RED, Color_END);
        fail_counter++;
    }

    return fail_counter;
}

/* runs the greater than factor tests */
int greater_than_factor_tests() {
    int fail_counter = 0;

    printf("Running greater-than factor tests...\n");

    gaussian_t s = init_gaussian1D(0, 0);
    gaussian_t msg_f_to_s = init_gaussian1D(0, 0);
    gaussian_t msg_g_to_s = init_gaussian1D(0, 0);

    gaussian_factor_info_t f = { init_gaussian1D_from_mean_and_variance(1,1), &s, &msg_f_to_s };
    greater_than_factor_info_t g = { 0.0, &s, &msg_g_to_s };

    if (gaussian_factor_update(&f) != 1.0) {
        printf("%sFAIL%s: gaussian_factor_update(&f) != 1.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(greater_than_factor_update(&g) - 1.0448277182202785) > 1e-6) {
        printf("%sFAIL%s: greater_than_factor_update(&g) != 1.0448277182202785\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s)- 1.2875999709391783) > 1e-6) {
        printf("%sFAIL%s: mean(s) != 1.2875999709391783\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s) - 0.6296862857766055) > 1e-6) {
        printf("%sFAIL%s: variance(s) != 0.6296862857766055\n", Color_RED, Color_END);
        fail_counter++;
    }

    s = init_gaussian1D(0, 0);

    if (gaussian_factor_log_variable_norm(&f) != 0.0) {
        printf("%sFAIL%s: gaussian_factor_log_variable_norm(&f) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (greater_than_factor_log_variable_norm(&g) != -1.5273215487580363) {
        printf("%sFAIL%s: greater_than_factor_log_variable_norm(&g) != -1.5273215487580363\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(mean(s)- 1.2875999709391783) > 1e-6) {
        printf("%sFAIL%s: mean(s) != 1.2875999709391783\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (fabs(variance(s) - 0.6296862857766055) > 1e-6) {
        printf("%sFAIL%s: variance(s) != 0.6296862857766055\n", Color_RED, Color_END);
        fail_counter++;
    }

    if (greater_than_factor_log_factor_norm(&g) != 1.3545677697345866) {
        printf("%sFAIL%s: greater_than_factor_log_factor_norm(&g) != 1.3545677697345866\n", Color_RED, Color_END);
        fail_counter++;
    }

    return fail_counter;
}

/* runs the ranking graph tests */
int ranking_graph_tests() {
    int fail_counter = 0;

    printf("Running ranking graph tests...\n");

    gaussian_t urgency[3] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t latent_urgency[3] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t diffs[2] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0) };

    gaussian_t msg_from_f_to_urgency[3] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_g_to_latent_urgency[3] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_g_to_urgency[3] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_s_to_diffs[2] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_s_to_top_urgency[2] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_s_to_urgency[2] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0) };
    gaussian_t msg_from_h_to_diffs[2] = { init_gaussian1D(0, 0), init_gaussian1D(0, 0) };

    gaussian_factor_info_t f[3] = { { init_gaussian1D_from_mean_and_variance(0,1), &urgency[0], &msg_from_f_to_urgency[0] },
                                    { init_gaussian1D_from_mean_and_variance(0,1), &urgency[1], &msg_from_f_to_urgency[1] },
                                    { init_gaussian1D_from_mean_and_variance(0,1), &urgency[2], &msg_from_f_to_urgency[2] } };

    gaussian_mean_factor_info_t g[3] = { { 1./2 * 1./2, &latent_urgency[0], &urgency[0], &msg_from_g_to_latent_urgency[0], &msg_from_g_to_urgency[0] },
                                         { 1./2 * 1./2, &latent_urgency[1], &urgency[1], &msg_from_g_to_latent_urgency[1], &msg_from_g_to_urgency[1] },
                                         { 1./2 * 1./2, &latent_urgency[2], &urgency[2], &msg_from_g_to_latent_urgency[2], &msg_from_g_to_urgency[2] } };

    weighted_sum_factor_info_t s[2] = { { 1.0, -1.0, &latent_urgency[0], &latent_urgency[1], &diffs[0], &msg_from_s_to_top_urgency[0], &msg_from_s_to_urgency[0], &msg_from_s_to_diffs[0] },
                                        { 1.0, -1.0, &latent_urgency[0], &latent_urgency[2], &diffs[1], &msg_from_s_to_top_urgency[1], &msg_from_s_to_urgency[1], &msg_from_s_to_diffs[1] } };
    greater_than_factor_info_t h[2] = { { 0.0, &diffs[0], &msg_from_h_to_diffs[0] },
                                        { 0.0, &diffs[1], &msg_from_h_to_diffs[1] } };


    gaussian_factor_update(&f[0]);
    gaussian_factor_update(&f[1]);
    gaussian_factor_update(&f[2]);

    gaussian_mean_factor_update_to_variable(&g[0]);
    gaussian_mean_factor_update_to_variable(&g[1]);
    gaussian_mean_factor_update_to_variable(&g[2]);
    double delta = 1e4;
    while (delta > 1e-6) {
        delta = 0.0;

        delta = fmax(delta, weighted_sum_factor_update_to_sum(&s[0]));
        delta = fmax(delta, greater_than_factor_update(&h[0]));
        delta = fmax(delta, weighted_sum_factor_update_to_summand1(&s[0]));
        delta = fmax(delta, weighted_sum_factor_update_to_summand2(&s[0]));

        delta = fmax(delta, weighted_sum_factor_update_to_sum(&s[1]));
        delta = fmax(delta, greater_than_factor_update(&h[1]));
        delta = fmax(delta, weighted_sum_factor_update_to_summand1(&s[1]));
        delta = fmax(delta, weighted_sum_factor_update_to_summand2(&s[1]));
    }

    gaussian_mean_factor_update_to_mean(&g[0]);
    gaussian_mean_factor_update_to_mean(&g[1]);
    gaussian_mean_factor_update_to_mean(&g[2]);

    /* compute the log-normalization constant */
    urgency[0] = init_gaussian1D(0, 0);
    urgency[1] = init_gaussian1D(0, 0);
    urgency[2] = init_gaussian1D(0, 0);
    latent_urgency[0] = init_gaussian1D(0, 0);
    latent_urgency[1] = init_gaussian1D(0, 0);
    latent_urgency[2] = init_gaussian1D(0, 0);
    diffs[0] = init_gaussian1D(0, 0);
    diffs[1] = init_gaussian1D(0, 0);

    double logZ = 0.0;
    logZ += gaussian_factor_log_variable_norm(&f[0]);
    logZ += gaussian_factor_log_variable_norm(&f[1]);
    logZ += gaussian_factor_log_variable_norm(&f[2]);
    logZ += gaussian_mean_factor_log_variable_norm(&g[0]);
    logZ += gaussian_mean_factor_log_variable_norm(&g[1]);
    logZ += gaussian_mean_factor_log_variable_norm(&g[2]);
    logZ += weighted_sum_factor_log_variable_norm(&s[0]);
    logZ += weighted_sum_factor_log_variable_norm(&s[1]);
    logZ += greater_than_factor_log_variable_norm(&h[0]);
    logZ += greater_than_factor_log_variable_norm(&h[1]);
    
    logZ += gaussian_factor_log_factor_norm(&f[0]);
    logZ += gaussian_factor_log_factor_norm(&f[1]);
    logZ += gaussian_factor_log_factor_norm(&f[2]);
    logZ += gaussian_mean_factor_log_factor_norm(&g[0]);
    logZ += gaussian_mean_factor_log_factor_norm(&g[1]);
    logZ += gaussian_mean_factor_log_factor_norm(&g[2]);
    logZ += weighted_sum_factor_log_factor_norm(&s[0]);
    logZ += weighted_sum_factor_log_factor_norm(&s[1]);
    logZ += greater_than_factor_log_factor_norm(&h[0]);
    logZ += greater_than_factor_log_factor_norm(&h[1]);
    
    /* output the computation results */
    printf("\n\nThree Move example\n=================\n");
    printf("urgency[0] = %f +/- %f\n", mean(urgency[0]), sqrt(variance(urgency[0])));
    printf("urgency[1] = %f +/- %f\n", mean(urgency[1]), sqrt(variance(urgency[1])));
    printf("urgency[2] = %f +/- %f\n", mean(urgency[2]), sqrt(variance(urgency[2])));
    printf("P = %f\n", exp(logZ));

    return fail_counter;
}


/* runs the ranking graph tests */
int ranking_graph_tests2() {
    int fail_counter = 0;

    // printf("Running ranking graph tests (again)...\n");

    // gaussian_t urgency_ht[3] = { init_gaussian1D(0, 1), init_gaussian1D(0, 1), init_gaussian1D(0, 1) };

    // initalize_ranking_updates();
    // update(urgency_ht, (int[]){ 0, 1, 2 }, 3, 0.5 * 0.5);

    // printf("\n\nThree Move example\n=================\n");
    // printf("urgency[0] = %f +/- %f\n", mean(urgency_ht[0]), sqrt(variance(urgency_ht[0])));
    // printf("urgency[1] = %f +/- %f\n", mean(urgency_ht[1]), sqrt(variance(urgency_ht[1])));
    // printf("urgency[2] = %f +/- %f\n", mean(urgency_ht[2]), sqrt(variance(urgency_ht[2])));

    return fail_counter;
}




/*
 * MAIN ENTRY POINT
 */
int main() {
    int fail_counter = 0;

    // tests the Gaussian implementation
    fail_counter += gaussian_tests();

    // tests the Factor implementation
    fail_counter += gaussian_factor_tests();
    fail_counter += gaussian_mean_factor_tests();
    fail_counter += weighted_sum_factor_tests();
    fail_counter += greater_than_factor_tests();
    
    // tests the ranking graph
    fail_counter += ranking_graph_tests();
    fail_counter += ranking_graph_tests2();

    // notify if testing was successful
    if (fail_counter) {
        printf("%sFAILS: %d%s\n", Color_RED, fail_counter, Color_END);
        exit(EXIT_FAILURE);
    } else {
        printf("%sALL OK...%s\n\n", Color_GREEN, Color_END);
        exit(EXIT_SUCCESS);
    }
}

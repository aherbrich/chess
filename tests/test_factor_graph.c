#include <stdio.h>

#include "../include/gaussian.h"
#include "../include/chess.h"

board_t *OLDSTATE[MAXPLIES];
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

    gaussian1D_t g1 = init_gaussian1D_standard_normal();
    if (absdiff(gaussian1D_mult(g1,g1), init_gaussian1D(0, 2)) != 0.0) {
        printf("%sFAIL%s: absdiff(gaussian1D_mult(g1,g1), init_gaussian1D(0, 2)) != 0.0\n", Color_RED, Color_END);
        fail_counter++;
    }

    gaussian1D_t g2 = init_gaussian1D(0,0.5);
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

/*
 * MAIN ENTRY POINT
 */
int main() {
    int fail_counter = 0;

    // tests the Gaussian implementation
    fail_counter += gaussian_tests();

    // notify if testing was successful
    if (fail_counter) {
        printf("%sFAILS: %d%s\n", Color_RED, fail_counter, Color_END);
        exit(EXIT_FAILURE);
    } else {
        printf("%sALL OK...%s\n\n", Color_GREEN, Color_END);
        exit(EXIT_SUCCESS);
    }
}

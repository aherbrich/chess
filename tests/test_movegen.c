#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sys/time.h"

#include "../include/chess.h"
#include "../include/zobrist.h"

typedef struct _perfttest_t {
    char fen[256];
    int depths[16];
    int results[16];
} perfttest_t;

board_t *OLDSTATE[MAXPLIES];
uint64_t HISTORY_HASHES[MAXPLIES];
uint64_t GLOBAL_COUNT = 0; 

/* pads whitespaces left and right of given string until given width reached */
char *pad_to_center(char *str, int width) {
    // exit if given string is longer than given width
    if ((int)strlen(str) > width) {
        fprintf(stderr, "Given width smaller than string lentgh...");
        exit(1);
    }
    // determine buffer size and allocate memory
    int buffer_length = 128;
    while (width > buffer_length) {
        buffer_length *= 2;
    }
    char *buffer = (char *)malloc(sizeof(char) * buffer_length);

    // initialize buffer
    for (int i = 0; i < buffer_length; i++) {
        if (i < width) {
            // set all whitespaces upto given width
            buffer[i] = ' ';
        } else {
            // set rest of buffer with null terminator
            buffer[i] = '\0';
        }
    }

    int space_left = width - strlen(str);

    // copy string into the center of whitespaced buffer
    strcpy(buffer + (space_left / 2), str);
    // replace null terminator of copied string with whitespace
    buffer[space_left / 2 + strlen(str)] = ' ';

    return buffer;
}

/* prints perft test row */
void print_perft_test_row(char *fen, char *depth, char *expected, char *found,
                          char* nps ,char *passed, char *color) {
    // pad the info strings
    char *padded_fen = pad_to_center(fen, 84);
    char *padded_depth = pad_to_center(depth, 6);
    char *padded_expected = pad_to_center(expected, 10);
    char *padded_nps = pad_to_center(nps, 17);
    char *padded_found = pad_to_center(found, 10);
    char *padded_passed = pad_to_center(passed, 8);

    // print accordingly
    printf("| %s | %s | %s | %s | %s | %s%s%s |\n", padded_fen, padded_depth,
           padded_expected, padded_found, padded_nps, color, padded_passed, Color_END);

    // free padded strings
    free(padded_fen);
    free(padded_depth);
    free(padded_expected);
    free(padded_nps);
    free(padded_found);
    free(padded_passed);
}

/* prints perft test row seperator */
void print_perft_test_row_separator() {
    printf(
        "+---------------------------------------------------------------------"
        "-----------------+--------+------------+------------+-------------------+----------+\n");
}

/* determines number of lines in a file */
int count_lines_in_file() {
    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/perft_suite.txt");

    FILE *fp = fopen(file_name, "r");

    int line_count = 0;
    int character;
    do {
        character = fgetc(fp);
        if (character == '\n') line_count++;
    } while (character != EOF);
    fclose(fp);

    return line_count;
}

/* loads perft tests from file */
perfttest_t **load_perft_test_suite(int nr_of_tests) {
    // allocate memory for perft test data structures and initialize
    perfttest_t **perfttests =
        (perfttest_t **)malloc(sizeof(perfttest_t *) * nr_of_tests);
    for (int i = 0; i < nr_of_tests; i++) {
        perfttests[i] = NULL;
    }

    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/perft_suite.txt");

    // open file
    FILE *fp = fopen(file_name, "r");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (fp == NULL) {
        fprintf(stderr, "Perft test suite file doesnt exist...\n");
        exit(1);
    }

    // start parsing file
    int test_counter = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        // read (line by line) perft test data from file and save in perft test
        // datatable
        perfttest_t *perfttest = (perfttest_t *)malloc(sizeof(perfttest_t));
        perfttests[test_counter] = perfttest;
        test_counter++;

        for (int i = 0; i < 16; i++) {
            perfttest->depths[i] = -1;
            perfttest->results[i] = -1;
        }

        char *ptr = strtok(line, ";");
        strcpy(perfttest->fen, ptr);
        int idx = 0;
        while ((ptr = strtok(NULL, "; "))) {
            perfttest->depths[idx] = atoi(ptr + 1);
            ptr = strtok(NULL, "\n ");
            perfttest->results[idx] = atoi(ptr);
            ;
            idx++;
        }
    }
    free(line);
    fclose(fp);
    return perfttests;
}

/* runs a given perfttest */
int run_specific_test(perfttest_t *test) {
    board_t *board = init_board();
    load_by_FEN(board, test->fen);

    int nr_of_moves;
    struct timeval start;
    struct timeval end;

    // run perft test for given depths and output results
    for (int i = 0; i < 16 && test->results[i] != -1; i++) {
        char depth_str[32];
        char expected_str[32];
        char found_str[32];
        char nps_str[32];

        // perft result for given depth
        gettimeofday(&start, 0);
        nr_of_moves = perft(board, test->depths[i]);
        GLOBAL_COUNT += (uint64_t) nr_of_moves;
        gettimeofday(&end, 0);

        double nps = ((nr_of_moves)/((end.tv_sec-start.tv_sec)*1000.0 + (end.tv_usec-start.tv_usec)/1000.0))/1000.0;

        // string formatting and output
        sprintf(depth_str, "%d", test->depths[i]);
        sprintf(expected_str, "%d", test->results[i]);
        sprintf(found_str, "%d", nr_of_moves);
        sprintf(nps_str, "%.2f Mn/s", nps);

        if (test->results[i] == nr_of_moves) {
            print_perft_test_row(test->fen, depth_str, expected_str, found_str, nps_str,
                                 "yes", Color_GREEN);
        } else {
            print_perft_test_row(test->fen, depth_str, expected_str, found_str, nps_str,
                                 "no", Color_RED);
        }
        print_perft_test_row_separator();
    }
    free_board(board);

    return 0;
}

/*
 * MAIN ENTRY POINT
 */
int main() {
    // intialize necessary structures
    initialize_chess_engine_only_necessary();
    initialize_zobrist_table();
    // determine number of tests in file
    int nr_of_tests = count_lines_in_file();

    // load test suite
    perfttest_t **perfttests = load_perft_test_suite(nr_of_tests);

    // print header
    print_perft_test_row_separator();
    print_perft_test_row("fen", "depth", "expected", "found", "nps", "passed",
                         Color_WHITE);
    print_perft_test_row_separator();

    // start testing and output the results
    int fail_counter = 0;

    struct timeval global_start;
    struct timeval global_end;

    gettimeofday(&global_start, 0);
    for (int i = 0; i < nr_of_tests && perfttests[i]; i++) {
        fail_counter += run_specific_test(perfttests[i]);
        free(perfttests[i]);
    }
    gettimeofday(&global_end, 0);


    printf("Average nps: %.2f Mn/s\n", ((GLOBAL_COUNT)/((global_end.tv_sec-global_start.tv_sec)*1000.0 + (global_end.tv_usec-global_start.tv_usec)/1000.0))/1000.0);

    free(perfttests);

    // notify if testing was successful
    if (fail_counter) {
        printf("%sFAILS: %d%s\n", Color_RED, fail_counter, Color_END);
        exit(EXIT_FAILURE);
    } else {
        printf("%sALL OK...%s\n\n", Color_GREEN, Color_END);
        exit(EXIT_SUCCESS);
    }
}

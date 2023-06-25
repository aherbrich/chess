#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct _matrix_t{
    double* value;
    int64_t row;
    int64_t col;
} matrix_t;

typedef struct _thread_data_t{
    matrix_t* X;
    matrix_t* Y;
    matrix_t* res;

    int from_row;
    int until_row;
    int from_col;
    int until_col;
    

    int thread_id;
    int is_complete;
    double progress;
} thread_data_t;

matrix_t* matrix_init(int64_t n, int64_t m);
void matrix_free(matrix_t* matrix);
double matrix_read(matrix_t* X, int64_t i, int64_t j);
double matrix_read_as_transpose(matrix_t* X, int64_t i, int64_t j);
void matrix_set(matrix_t* X, double value, int64_t i, int64_t j);
void matrix_set_as_transpose(matrix_t* X, double value, int64_t i, int64_t j);
void matrix_print(matrix_t* X);

matrix_t* matrix_transpose(matrix_t* X);
matrix_t* matrix_mult_first_arg_transposed(matrix_t* X, matrix_t* Y);
matrix_t* matrix_addition(matrix_t* X, matrix_t* Y);
void matrix_regularize(matrix_t* X, double lambda);
matrix_t* matrix_mult(matrix_t* X, matrix_t* Y);
matrix_t* matrix_mult_gram(matrix_t* X);

matrix_t* cholesky(matrix_t* X);
matrix_t* back_substitution(matrix_t* X, matrix_t* b, int for_transpose);
matrix_t* forward_substitution(matrix_t* X, matrix_t* b, int for_transpose);
matrix_t* solve_cholesky(matrix_t* X, matrix_t* b);
void cholesky_N_threaded(matrix_t* X, int max_threads);
matrix_t* solve_cholesky_threaded(matrix_t* X, matrix_t* b, int max_threads);

void compute_nr_of_threads(int max_threads, int* nr_threads, int* blocks);
void initialize_thread_data(thread_data_t* thread_data, matrix_t* X, matrix_t* Y, matrix_t* result, int from_row, int until_row, int from_col, int until_col, int thread_id);
void* multiplication_gram(void *threadarg);
matrix_t* matrix_mult_gram_N_threaded(matrix_t* X, int max_threads);


#endif
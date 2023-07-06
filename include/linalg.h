#ifndef __LINALG_H__
#define __LINALG_H__

#include <stdint.h>
#include <stdlib.h>

typedef struct _matrix_t {
    double* value;
    int64_t row;
    int64_t col;
} matrix_t;

/////////////////////////////////////////////////////////////
//  BASIC OPERATIONS
matrix_t* matrix_init(int64_t n, int64_t m);
matrix_t* matrix_init_like(matrix_t* X);
void matrix_free(matrix_t* matrix);

double matrix_read(matrix_t* X, int64_t i, int64_t j);
double matrix_read_as_transpose(matrix_t* X, int64_t i, int64_t j);
void matrix_set(matrix_t* X, double value, int64_t i, int64_t j);
void matrix_set_as_transpose(matrix_t* X, double value, int64_t i, int64_t j);

matrix_t* matrix_copy(matrix_t* X);

void matrix_print(matrix_t* X);

/////////////////////////////////////////////////////////////
//  TRANSPOSE
matrix_t* matrix_transpose(matrix_t* X);
matrix_t* matrix_transpose_square_inplace(matrix_t* X);

/////////////////////////////////////////////////////////////
//  ADDITION
matrix_t* matrix_addition(matrix_t* X, matrix_t* Y);
matrix_t* matrix_addition_inplace(matrix_t* X, matrix_t* Y, int free_Y);

matrix_t* matrix_regularize(matrix_t* X, double lambda);
matrix_t* matrix_regularize_inplace(matrix_t* X, double lambda);

/////////////////////////////////////////////////////////////
//  MULTIPLICATION
matrix_t* matrix_mult(matrix_t* X, matrix_t* Y);
matrix_t* matrix_mult_first_arg_transposed(matrix_t* X, matrix_t* Y);
matrix_t* matrix_mult_gram(matrix_t* X);
matrix_t* matrix_mult_gram_threaded(matrix_t* X, int max_threads);

/////////////////////////////////////////////////////////////
//  CHOLESKY
matrix_t* cholesky(matrix_t* X);
matrix_t* cholesky_inplace(matrix_t* X);
matrix_t* cholesky_outer_inplace(matrix_t* X);
matrix_t* cholesky_outer_inplace_threaded(matrix_t* X, int max_threads);

/////////////////////////////////////////////////////////////
//  SOLVE
matrix_t* solve_cholesky(matrix_t* X, matrix_t* b);
matrix_t* solve_cholesky_threaded(matrix_t* X, matrix_t* b, int max_threads);

#endif
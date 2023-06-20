#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct _matrix_t{
    double* value;
    int64_t row;
    int64_t col;
} matrix_t;

matrix_t* matrix_init(int64_t n, int64_t m){
    matrix_t* matrix = (matrix_t* ) malloc(sizeof(matrix_t));
    matrix->value = (double* ) calloc(n * m, sizeof(double));
    matrix->row = n;
    matrix->col = m;
    return matrix;
}

void matrix_free(matrix_t* matrix){
    free(matrix->value);
    free(matrix);
}

double matrix_read(matrix_t* A, int64_t i, int64_t j){
    int64_t idx = j*A->row+i;
    return A->value[idx];
}

double matrix_read_as_transpose(matrix_t* A, int64_t i, int64_t j){
    int64_t idx = i*A->col+j;
    return A->value[idx];
}

void matrix_set(matrix_t* A, double value, int64_t i, int64_t j){
    int64_t idx = j*A->row+i;
    A->value[idx] = value;
}

void matrix_set_as_transpose(matrix_t* A, double value, int64_t i, int64_t j){
    int64_t idx = i*A->col+j;
    A->value[idx] = value;
}

void matrix_print(matrix_t* A){
    for(int i = 0;  i < A->row; i++){
        for(int j = 0; j < A->col; j++){
            printf("%f ", matrix_read(A, i, j));
        }
        printf("\n");
    }
    printf("\n");
}

matrix_t* matrix_transpose(matrix_t* A){
    matrix_t* result = matrix_init(A->col, A->row);
    for(int i = 0;  i < A->row; i++){
        for(int j = 0; j < A->col; j++){
            matrix_set(result, matrix_read(A, i, j), j, i);
        }
    }
    return result;
}

matrix_t* matrix_mult(matrix_t* A, matrix_t* B){
    if(A->col != B->row){
        perror("MULTIPLICATION - Dimension mismatch!");
    }

    matrix_t* result = matrix_init(A->row, B->col);
    for(int i = 0; i < result->row; i++){
        for(int j = 0; j < result->col; j++){
            double val = 0;
            for(int k = 0; k < A->col; k++){
                val += matrix_read(A, i, k) * matrix_read(B, k, j);
            }
            matrix_set(result, val, i, j);
        }
    }

    return result;
}

matrix_t* matrix_addition(matrix_t* A, matrix_t* B){
    if(A->col != B->col || A->row != B->row){
        perror("ADDITION - Dimension mismatch!");
    }

    matrix_t* result = matrix_init(A->col, A->row);
    for(int i = 0; i < result->row; i++){
        for(int j = 0; j < result->col; j++){
            matrix_set(result, matrix_read(A, i, j) + matrix_read(B, i, j), i, j);
        }
    }

    return result;
}

matrix_t* matrix_mult_identical(matrix_t* A, int transpose_first){
    if(transpose_first){
        matrix_t* result = matrix_init(A->col, A->col);
        for(int i = 0; i < result->row; i++){
            for(int j = i; j < result->col; j++){
                double val = 0.0;
                for(int k = 0; k < A->row; k++){
                    val += matrix_read(A, k, i) * matrix_read(A, k, j);
                }
                matrix_set(result, val, i, j);
                matrix_set(result, val, j, i);
            }
        }

        return result;
    } else{
        matrix_t* result = matrix_init(A->row, A->row);
        double res = 0;
        for(int i = 0; i < result->row; i++){
            for(int j = i; j < result->col; j++){
                double val = 0.0;
                for(int k = 0; k < A->col; k++){
                    val += matrix_read(A, i, k) * matrix_read(A, j, k);
                }
                matrix_set(result, val, i, j);
                matrix_set(result, val, j, i);
            }
        }

        return result;
    }
}

matrix_t* cholesky(matrix_t* A){
    int n = A->row;
    int m = A->col;

    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(matrix_read(A, i, j) != matrix_read_as_transpose(A, i, j)){
                matrix_print(A);
                perror("CHOLESKY - Matrix not symmetric!");
            }
        }
    }

    matrix_t* L = matrix_init(n, n);

    for(int i = 0; i < n; i++){
        for(int j = 0; j < i+1; j++){
            if(i == j){
                double l_ii = matrix_read(A, i, i);
                for(int k = 0; k < i; k++){
                    l_ii -= (matrix_read(L, i, k)*matrix_read(L, i, k));
                }
                if(l_ii < 0){
                    perror("CHOLESKY - Matrix not PSD!");
                }
                    
                l_ii = sqrt(l_ii);
                matrix_set(L, l_ii, i, i);
            } else{
                if (matrix_read(L, j, j) == 0){
                    perror("CHOLESKY - Matrix not PSD!");
                }
                
                double l_ij = matrix_read(A, i, j);
                for(int k = 0; k < j; k++){
                    l_ij -= (matrix_read(L, i, k) * matrix_read(L, j, k));
                }
                    

                l_ij = l_ij/matrix_read(L, j,j);

                matrix_set(L, l_ij, i, j);
            }
                
        }
    }
    return L;
}

matrix_t* back_substitution(matrix_t* A, matrix_t* b, int for_transpose){
    if((A->row != A->col) || (A->col != b->row)){
        perror("BACKSUBSTITUTION - Dimension mismatch");
    }
    
    matrix_t* x =  matrix_init(b->row, b->col);

    if(!for_transpose){
        for(int i = b->row-1; i >= 0; i--){
            double x_i = matrix_read(b, i, 0);
            for(int j = i+1; j < b->row; j++){
                x_i -= matrix_read(x, j, 0) * matrix_read(A, i, j);
            }

            if(matrix_read(A, i, i) == 0){
                perror("BACKSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
            }
                
            x_i = x_i/matrix_read(A, i, i);
            matrix_set(x, x_i, i, 0);  
        }
    } else{
        for(int i = b->row-1; i >= 0; i--){
            double x_i = matrix_read_as_transpose(b, i, 0);
            for(int j = i+1; j < b->row; j++){
                x_i -= matrix_read_as_transpose(x, j, 0) * matrix_read_as_transpose(A, i, j);
            }

            if(matrix_read_as_transpose(A, i, i) == 0){
                perror("BACKSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
            }
                
            x_i = x_i/matrix_read_as_transpose(A, i, i);
            matrix_set_as_transpose(x, x_i, i, 0);  
        }
    }
    
    
    return x;
}

matrix_t* forward_substitution(matrix_t* A, matrix_t* b, int for_transpose){
    if((A->row != A->col) || (A->col != b->row)){
        perror("FORWARDSUBSTITUTION - Dimension mismatch");
    }

    matrix_t* x =  matrix_init(b->row, b->col);

    if(!for_transpose){
        for(int i = 0; i < b->row; i++){
            double x_i = matrix_read(b, i, 0);

            for(int j = 0; j < i; j++){
                x_i -= (matrix_read(A, i, j) * matrix_read(x, j, 0));
            }
            
            if(matrix_read(A, i, i) == 0){
                perror("FORWARDSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
            }
            
            x_i = x_i/matrix_read(A, i, i);
            matrix_set(x, x_i, i, 0);
        }
    } else{
        for(int i = 0; i < b->row; i++){
            double x_i = matrix_read_as_transpose(b, i, 0);

            for(int j = 0; j < i; j++){
                x_i -= (matrix_read_as_transpose(A, i, j) * matrix_read_as_transpose(x, j, 0));
            }
            
            if(matrix_read_as_transpose(A, i, i) == 0){
                perror("FORWARDSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
            }
            
            x_i = x_i/matrix_read_as_transpose(A, i, i);
            matrix_set_as_transpose(x, x_i, i, 0);
        }
    }
    
    
    return x;
}

matrix_t* solve_cholesky(matrix_t* A, matrix_t* b){
    matrix_t* L = cholesky(A);

    matrix_t* y = forward_substitution(L, b, 0);
    matrix_t* x = back_substitution(L, y, 1);

    matrix_free(L);
    matrix_free(y);

    return x;
}
#include "../include/matrix.h"

/* Initializes matrix */
matrix_t* matrix_init(int64_t n, int64_t m){
    matrix_t* matrix = (matrix_t* ) malloc(sizeof(matrix_t));
    matrix->value = (double* ) calloc(n * m, sizeof(double));
    matrix->row = n;
    matrix->col = m;
    return matrix;
}

/* Frees memory for matrix */
void matrix_free(matrix_t* matrix){
    free(matrix->value);
    free(matrix);
}

/* Reads entry in the i'th row and j'th column */
double matrix_read(matrix_t* X, int64_t i, int64_t j){
    int64_t idx = j*X->row+i;
    return X->value[idx];
}

/* Reads entry in the i'th column and j'th row */
double matrix_read_as_transpose(matrix_t* X, int64_t i, int64_t j){
    int64_t idx = i*X->col+j;
    return X->value[idx];
}

/* Sets entry in the i'th row and j'th column */
void matrix_set(matrix_t* X, double value, int64_t i, int64_t j){
    int64_t idx = j*X->row+i;
    X->value[idx] = value;
}

/* Sets entry in the i'th column and j'th row */
void matrix_set_as_transpose(matrix_t* X, double value, int64_t i, int64_t j){
    int64_t idx = i*X->col+j;
    X->value[idx] = value;
}

/* Matrix pretty print  */
void matrix_print(matrix_t* X){
    for(int i = 0;  i < X->row; i++){
        for(int j = 0; j < X->col; j++){
            fprintf(stderr, "%.3f\t", matrix_read(X, i, j));
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

/* Transposes a given matrix */
/* WARNING: not inplace */
/* Memory for X.T will be allocated, and matrix X will NOT be freed */
matrix_t* matrix_transpose(matrix_t* X){
    matrix_t* result = matrix_init(X->col, X->row);
    for(int i = 0;  i < X->row; i++){
        for(int j = 0; j < X->col; j++){
            matrix_set(result, matrix_read(X, i, j), j, i);
        }
    }
    return result;
}

/* Adds to matrices */
/* WARNING: not inplace */
/* Memory for X+Y will be allocated, and matrix X,Y will NOT be freed */
matrix_t* matrix_addition(matrix_t* X, matrix_t* Y){
    if(X->col != Y->col || X->row != Y->row){
        perror("ADDITION - Dimension mismatch!");
        exit(EXIT_FAILURE);
    }

    matrix_t* result = matrix_init(X->col, X->row);
    for(int i = 0; i < result->row; i++){
        for(int j = 0; j < result->col; j++){
            matrix_set(result, matrix_read(X, i, j) + matrix_read(Y, i, j), i, j);
        }
    }

    return result;
}

/* Adds to matrices */
/* WARNING: Inplace operation */
void matrix_regularize(matrix_t* X, double lambda){
    if(X->col != X->row){
        perror("REGULARIZATION - Matrix not sqaure!");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < X->row; i++){
        double val = matrix_read(X, i, i);
        matrix_set(X, val+lambda, i, i);
    }
}

/* Multiplies two matrices (the old fashioned way) */
/* WARNING: not inplace */
/* Memory for X*Y will be allocated, and matrix X,Y will NOT be freed */
matrix_t* matrix_mult(matrix_t* X, matrix_t* Y){
    if(X->col != Y->row){
        perror("MULTIPLICATION - Dimension mismatch!");
        exit(EXIT_FAILURE);
    }

    matrix_t* result = matrix_init(X->row, Y->col);
    for(int i = 0; i < result->row; i++){
        for(int j = 0; j < result->col; j++){
            double val = 0;
            for(int k = 0; k < X->col; k++){
                val += matrix_read(X, i, k) * matrix_read(Y, k, j);
            }
            matrix_set(result, val, i, j);
        }
    }

    return result;
}

/* Multiplies two matrices (the old fashioned way) */
/* WARNING: not inplace */
/* Memory for X.T*Y will be allocated, and matrix X,Y will NOT be freed */
/* BONUS: Transpose of X happens inplace */
matrix_t* matrix_mult_first_arg_transposed(matrix_t* X, matrix_t* Y){
    if(X->row != Y->row){
        perror("MULTIPLICATION - Dimension mismatch!");
        exit(EXIT_FAILURE);
    }

    matrix_t* result = matrix_init(X->col, Y->col);
    for(int i = 0; i < result->row; i++){
        for(int j = 0; j < result->col; j++){
            double val = 0;
            for(int k = 0; k < X->row; k++){
                val += matrix_read(X, k, i) * matrix_read(Y, k, j);
            }
            matrix_set(result, val, i, j);
        }
    }

    return result;
}

/* Multiplies a given matrix with itsself (Gram matrix) */
/* WARNING: not inplace */
/* Memory for X.T*X will be allocated, and matrix X will NOT be freed */
/* BONUS: Faster and more efficient than (normal) matrix multiplication */
matrix_t* matrix_mult_gram(matrix_t* X){
    matrix_t* result = matrix_init(X->col, X->col);
        
    for(int i = 0; i < result->row; i++){
        for(int j = i; j < result->col; j++){
            double val = 0.0;
            for(int k = 0; k < X->row; k++){
                val += matrix_read(X, k, i) * matrix_read(X, k, j);
            }
            matrix_set(result, val, i, j);
            matrix_set(result, val, j, i);
        }
    }

    return result;
}

/* Determines Cholesky decomposition of a given (PSD) matrix */
/* WARNING: not inplace */
/* Memory for L will be allocated, and matrix X will NOT be freed */
matrix_t* cholesky(matrix_t* X){
    int n = X->row;
    int m = X->col;

    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(matrix_read(X, i, j) != matrix_read_as_transpose(X, i, j)){
                matrix_print(X);
                perror("CHOLESKY - Matrix not symmetric!");
                exit(EXIT_FAILURE);
            }
        }
    }

    matrix_t* L = matrix_init(n, n);

    for(int i = 0; i < n; i++){
        for(int j = 0; j < i+1; j++){
            if(i == j){
                double l_ii = matrix_read(X, i, i);
                for(int k = 0; k < i; k++){
                    l_ii -= (matrix_read(L, i, k)*matrix_read(L, i, k));
                }
                if(l_ii < 0){
                    perror("CHOLESKY - Matrix not PSD!");
                    exit(EXIT_FAILURE);
                }
                    
                l_ii = sqrt(l_ii);
                matrix_set(L, l_ii, i, i);
            } else{
                if (matrix_read(L, j, j) == 0){
                    perror("CHOLESKY - Matrix not PSD!");
                    exit(EXIT_FAILURE);
                }
                
                double l_ij = matrix_read(X, i, j);
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

/* Backsubstitutes a upper triangular matrix */
/* Alternatively, a lower triangular matrix can be passed, which will then be transposed inplace*/
/* WARNING: not inplace */
/* Memory for x will be allocated, and matrix X,b will NOT be freed */
matrix_t* back_substitution(matrix_t* X, matrix_t* b, int for_transpose){
    if((X->row != X->col) || (X->col != b->row)){
        perror("BACKSUBSTITUTION - Dimension mismatch");
        exit(EXIT_FAILURE);
    }
    
    matrix_t* x =  matrix_init(b->row, b->col);

    if(!for_transpose){
        for(int i = b->row-1; i >= 0; i--){
            double x_i = matrix_read(b, i, 0);
            for(int j = i+1; j < b->row; j++){
                x_i -= matrix_read(x, j, 0) * matrix_read(X, i, j);
            }

            if(matrix_read(X, i, i) == 0){
                perror("BACKSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
                exit(EXIT_FAILURE);
            }
                
            x_i = x_i/matrix_read(X, i, i);
            matrix_set(x, x_i, i, 0);  
        }
    } else{
        for(int i = b->row-1; i >= 0; i--){
            double x_i = matrix_read_as_transpose(b, i, 0);
            for(int j = i+1; j < b->row; j++){
                x_i -= matrix_read_as_transpose(x, j, 0) * matrix_read_as_transpose(X, i, j);
            }

            if(matrix_read_as_transpose(X, i, i) == 0){
                perror("BACKSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
                exit(EXIT_FAILURE);
            }
                
            x_i = x_i/matrix_read_as_transpose(X, i, i);
            matrix_set_as_transpose(x, x_i, i, 0);  
        }
    }
    
    
    return x;
}

/* Forwardsubstitutes a lower triangular matrix */
/* Alternatively, an upper triangular matrix can be passed, which will then be transposed inplace*/
/* WARNING: not inplace */
/* Memory for x will be allocated, and matrix X,b will NOT be freed */
matrix_t* forward_substitution(matrix_t* X, matrix_t* b, int for_transpose){
    if((X->row != X->col) || (X->col != b->row)){
        perror("FORWARDSUBSTITUTION - Dimension mismatch");
        exit(EXIT_FAILURE);
    }

    matrix_t* x =  matrix_init(b->row, b->col);

    if(!for_transpose){
        for(int i = 0; i < b->row; i++){
            double x_i = matrix_read(b, i, 0);

            for(int j = 0; j < i; j++){
                x_i -= (matrix_read(X, i, j) * matrix_read(x, j, 0));
            }
            
            if(matrix_read(X, i, i) == 0){
                perror("FORWARDSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
                exit(EXIT_FAILURE);
            }
            
            x_i = x_i/matrix_read(X, i, i);
            matrix_set(x, x_i, i, 0);
        }
    } else{
        for(int i = 0; i < b->row; i++){
            double x_i = matrix_read_as_transpose(b, i, 0);

            for(int j = 0; j < i; j++){
                x_i -= (matrix_read_as_transpose(X, i, j) * matrix_read_as_transpose(x, j, 0));
            }
            
            if(matrix_read_as_transpose(X, i, i) == 0){
                perror("FORWARDSUBSTITUTION - Not diagonal/zero diagonal element. Please regularize!");
                exit(EXIT_FAILURE);
            }
            
            x_i = x_i/matrix_read_as_transpose(X, i, i);
            matrix_set_as_transpose(x, x_i, i, 0);
        }
    }
    
    
    return x;
}

/* Solves the system of linear equations (Xw=b) for w by cholesky decompostion */
/* X = m x m and PSD;  b = m x 1 */
/* WARNING: not inplace */
/* Memory for w will be allocated, and matrix X,b will NOT be freed */
matrix_t* solve_cholesky(matrix_t* X, matrix_t* b){
    matrix_t* L = cholesky(X);
    fprintf(stderr, "Solved LLT!\n");

    matrix_t* y = forward_substitution(L, b, 0);
    fprintf(stderr, "Solved forward subsitution!\n");
    matrix_t* w = back_substitution(L, y, 1);
    fprintf(stderr, "Solved backward subsitution!\n");

    matrix_free(L);
    matrix_free(y);

    return w;
}

///////////////////////////////////
//  THREADED OPERATIONS

/* Initializes thread data struct */
void initialize_thread_data(thread_data_t* thread_data, matrix_t* X, matrix_t* Y, matrix_t* result, int from_row, int until_row, int from_col, int until_col, int thread_id){
    thread_data->X = X;
    thread_data->Y = Y;
    thread_data->res = result;

    thread_data->from_row = from_row;
    thread_data->until_row = until_row;
    thread_data->from_col = from_col;
    thread_data->until_col = until_col;
    
    thread_data->thread_id = thread_id;
    thread_data->is_complete = 0;
    thread_data->progress = 0.0;

    fprintf(stderr, "[Thread %d] (%d,%d)=%d (%d, %d)=%d\n", thread_id, from_row, until_row, until_row-from_row, from_col, until_col, until_col-from_col);
}

/* Computes the optimal number of threads used in the computation of the gram matrix */
/* Given the maximum number of threads available */
void compute_nr_of_threads(int max_threads, int* nr_threads, int* blocks){
    int old_res = 0;
    int res = 0;

    int old_counter = 0;
    int counter = 0;

    int inc = 1;
    while(res <= max_threads){
        old_res = res;
        res += inc;

        old_counter = counter;
        counter++;

        inc++;  
    }

    *nr_threads = old_res;
    *blocks = old_counter;
}

/* Computes a section/block of the gram matrix */
/* Section being defined by start and end idx of row and columns of the Gram matrix */
void* multiplication_gram(void *threadarg)
{   
    thread_data_t *my_data;
    my_data = (thread_data_t *) threadarg;

    // multiplication of diagonal block
    if(my_data->from_row == my_data->from_col){
        for(int i = my_data->from_row; i < my_data->until_row; i++){
            my_data->progress = ((float) i-my_data->from_row)/((float)(my_data->until_row-my_data->from_row));
            for(int j = i; j < my_data->until_col; j++){
                double val = 0.0;
                for(int k = 0; k < my_data->X->row; k++){
                    val += matrix_read(my_data->X, k, i) * matrix_read(my_data->X, k, j);
                }
                matrix_set(my_data->res, val, i, j);
                matrix_set(my_data->res, val, j, i);
            }
        }
        my_data->is_complete = 1;
    } 
    // multiplication of non diagonal block
    else{
        for(int i = my_data->from_row; i < my_data->until_row; i++){
            my_data->progress = ((float) i-my_data->from_row)/((float)(my_data->until_row-my_data->from_row));
            for(int j = my_data->from_col; j < my_data->until_col; j++){
                double val = 0.0;
                for(int k = 0; k < my_data->X->row; k++){
                    val += matrix_read(my_data->X, k, i) * matrix_read(my_data->X, k, j);
                }
                matrix_set(my_data->res, val, i, j);
                matrix_set(my_data->res, val, j, i);
            }
        }
        my_data->is_complete = 1;
    }
    return NULL;
}

/* Computes the Gram matrix by multithreaded matrix multiplication */
/* Arg: Maximum number of threads available */
matrix_t* matrix_mult_gram_N_threaded(matrix_t* X, int max_threads){
    int nr_threads = 0;
    int blocks = 0;

    if(max_threads <= 1){
        perror("THREADED MULT - Can't do threaded multiply with only one thread!");
        exit(EXIT_FAILURE);
    }

    compute_nr_of_threads(max_threads-1, &nr_threads, &blocks);

    if(blocks > X->col){
        perror("THREADED MULT - More threads specified than could be possibly used!");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[nr_threads];
    thread_data_t thread_data_array[nr_threads];

    int middle = X->col/blocks;


    // compute and initiliaze partial matrice boundaries
    matrix_t* result = matrix_init(X->col, X->col);

    int indices[blocks+1];
    for(int i = 0; i < blocks+1; i++){
        indices[i] = i * middle;
    }
    indices[blocks] = X->col;

    int idx = 0;
    for(int i = 0; i < blocks; i++){
        for(int j = i; j < blocks; j++){
            initialize_thread_data(&thread_data_array[idx], X, NULL, result, indices[i], indices[i+1], indices[j], indices[j+1], idx);
            idx++;
        }
    }

    // start threads
    for(int i=0; i<nr_threads; i++){
        int rc =pthread_create (&threads[i], NULL, multiplication_gram, (void *) &thread_data_array[i]);
        if (rc){
            fprintf(stderr, "ERROR; return code from () is %d\n", rc);
            exit(-1);
        }
    }

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, 0);

    int finished_threads = 0;
    while(finished_threads != nr_threads){
        gettimeofday(&end, 0);
        if(end.tv_sec-start.tv_sec >= 60){
            gettimeofday(&start, 0);
            for(int i = 0; i < nr_threads; i++){
                fprintf(stderr, " %.2f |", (&thread_data_array[i])->progress);
            }
            fprintf(stderr, "\n");
            for(int i = 0; i < nr_threads; i++){
                fprintf(stderr, "------+");
            }
            fprintf(stderr, "\n");
        }
        finished_threads = 0;
        for(int i = 0; i < nr_threads; i++){
            if((&thread_data_array[i])->is_complete) finished_threads++;
        }
    }

    // join threads
    for (int i = 0; i < nr_threads; i++){
        pthread_join(threads[i], NULL);
    }
    
    return result;
}
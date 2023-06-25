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
        perror("MULTIPLICATION TRANS - Dimension mismatch!");
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

matrix_t* cholesky_outer(matrix_t* X){
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

    for (int k = 0; k < n; k++) {
        // normalize k'th column
        double val = sqrt(matrix_read(X, k, k));
        for(int i = k; i < n; i++){
            matrix_set(X, matrix_read(X, i, k)/val, i, k);
        }

        // begin computation of 'outer prouct'
        for(int j = k+1; j < n; j++){
            for(int i = j; i < n; i++){
                val = matrix_read(X, i, j) - matrix_read(X, i, k) * matrix_read(X, j, k);
                matrix_set(X, val, i, j);
            }
        }
    }

    for(int j = 1; j < n; j++){
        for(int i = 0; i < j; i++){
            matrix_set(X, 0.0, i, j);
        }
    }

    return NULL;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//
//  THREADED OPERATIONS
//
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


typedef struct _thread_cholesky_t{
    matrix_t* X;

    int id;

    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* first_thread_finished;
    int* unblocked_after_first_finished;

    pthread_mutex_t* mutex2;
    pthread_cond_t* cond2;
    int* finished_threads;
    int* unblocked_after_all_finished;

    int nr_threads;
    int* indices;

} thread_cholesky_t;

void initialize_cholesky_thread_data(thread_cholesky_t* data, matrix_t* X, int id, pthread_mutex_t* mutex, pthread_mutex_t* mutex2, pthread_cond_t* cond, pthread_cond_t* cond2, int* flag1, int* flag2, int* flag3, int* flag4, int nr_threads, int* indices){
    data->X = X;

    data->id = id;

    data->mutex = mutex;
    data->cond = cond;
    data->first_thread_finished = flag1;
    data->unblocked_after_first_finished = flag2;

    data->mutex2 = mutex2;
    data->cond2 = cond2;
    data->finished_threads = flag3;
    data->unblocked_after_all_finished = flag4;

    data->nr_threads = nr_threads;
    data->indices = indices;
}

void* cholesky_thread(void* threadargs){
    thread_cholesky_t* data = (thread_cholesky_t*) threadargs;

    matrix_t* X = data->X;
    int thread_id = data->id;

    int n = X->row;
    int m = X->col;


    for (int k = 0; k < n; k++) {
        /* SNYNCHRONIZATION OF ALL THREADS (TO WAIT FOR THREAD 1) */
                /* THREAD 1 */
                if(thread_id == 0){
                    /* wait for all threads from last iteration to unblock */
                    pthread_mutex_lock(data->mutex2);
                    while(*(data->unblocked_after_all_finished) != 0){
                        //fprintf(stderr,"[%d] Locked thread! Waiting for all threads to proceed to next iteration!\n", thread_id);
                        pthread_cond_wait(data->cond2, data->mutex2);
                    }
                    pthread_mutex_unlock(data->mutex2);

                    /* PRECOMPUTATION IN THREAD 1 */
                    // normalize k'th column
                    //fprintf(stderr,"[%d] Doing computation! All threads have to wait for me to finish!\n", thread_id);
                    double val = sqrt(matrix_read(X, k, k));
                    for(int i = k; i < n; i++){
                        matrix_set(X, matrix_read(X, i, k)/val, i, k);
                    }
                    //fprintf(stderr,"[%d] Finished computation!\n", thread_id);
                    /* calaculate indice information (so that each thread knows what to compute)*/
                    for(int i = 0; i < data->nr_threads+1; i++){
                        data->indices[i] = 0; 
                    }

                    uint64_t nr_cols = n-(k+1);

                    if(nr_cols >= data->nr_threads){ 
                        uint64_t C = (uint64_t) ((nr_cols*(nr_cols+1))/data->nr_threads);
                        for(int i = 1; i < data->nr_threads+1; i++){
                            if(i == data->nr_threads){
                                data->indices[data->nr_threads] = (int) nr_cols;
                                break;
                            }

                            uint64_t j = data->indices[i-1]+1;
                            double bound = -0.5+sqrt(0.25-j+j*j+C);
                            int new_idx = ceil(bound);
                            data->indices[i] = new_idx; 

                            if(new_idx >= nr_cols){
                                data->indices[i] = nr_cols; 
                                break;
                            } 
                        }
                        
                    } else{
                        data->indices[1] = nr_cols;
                    }

                    for(int i = 0; i < data->nr_threads+1; i++){
                        data->indices[i] = nr_cols-data->indices[i]+k; 
                        //fprintf(stderr, "%d ", data->indices[i]);
                    }
                    //fprintf(stderr, "\n");

                    /* signal all other threads that they can continue computation */
                    pthread_mutex_lock(data->mutex);
                    //fprintf(stderr,"[%d] Unlocking threads!\n", thread_id);
                    *(data->first_thread_finished) = 1;
                    pthread_cond_broadcast(data->cond);
                    pthread_mutex_unlock(data->mutex);
                }
                /* THREAD OTHER (NOT 1)*/ 
                else{
                    /* wait for the first thread to finish (pre)computation */
                    pthread_mutex_lock(data->mutex);
                    while(!(*(data->first_thread_finished))){
                        //fprintf(stderr,"[%d] Locking thread, waiting for thread 1 to finish!\n", thread_id);
                        pthread_cond_wait(data->cond, data->mutex);
                        
                    }

                    /* the last thread which unblocks has to reset the conditionals */
                    (*(data->unblocked_after_first_finished))++;
                    if(*(data->unblocked_after_first_finished) == data->nr_threads-1){
                        //fprintf(stderr, "[%d] I am last to proceed to the second part of computation! I reset all conditionals!\n", thread_id);
                        *(data->unblocked_after_first_finished) = 0;
                        *(data->first_thread_finished) = 0;
                    }
                    //fprintf(stderr,"[%d] Unlocked thread, since thread 1 finished!\n", thread_id);

                    pthread_mutex_unlock(data->mutex);     
                }

        
        /* THE COMPUTATION */
        // begin computation of 'outer prouct'
        for(int j = data->indices[thread_id]; j > data->indices[thread_id+1]; j--){
            //fprintf(stderr, "[%d] Computing col %d\n", thread_id, j);
            for(int i = j; i < n; i++){
                double val = matrix_read(X, i, j) - matrix_read(X, i, k) * matrix_read(X, j, k);
                matrix_set(X, val, i, j);
            }
        }

        /* SNYNCHRONIZATION OF ALL THREADS (TO START NEXT ITEARTON SIMULTANEOUSLY) */

                pthread_mutex_lock(data->mutex2);
                (*(data->finished_threads))++;

                /* wait for the all threads to finish computation */
                while(*(data->finished_threads) != data->nr_threads){
                    //fprintf(stderr,"[%d] Locking thread, waiting for all to finish!\n", thread_id);
                    pthread_cond_wait(data->cond2, data->mutex2);
                }

                /* the last thread signals all other threads that they can continue to next iteration  */
                if(*(data->unblocked_after_all_finished)==0){
                    //fprintf(stderr, "[%d] I finished last, all others can continue now!\n", thread_id);
                    pthread_cond_broadcast(data->cond2);
                }   

                (*(data->unblocked_after_all_finished))++;
                
                /* the last thread which unblocks has to reset the conditionals */
                if(*(data->unblocked_after_all_finished) == data->nr_threads){
                    //fprintf(stderr, "[%d] I am last to proceed to next iteration! I reset all conditionals!\n", thread_id);
                    *(data->finished_threads) = 0;
                    *(data->unblocked_after_all_finished) = 0;
                    pthread_cond_signal(data->cond2);
                }
                //fprintf(stderr,"[%d] Unlocked thread, since all finished!\n", thread_id);
                pthread_mutex_unlock(data->mutex2);
    }

    /* set upper triangle zero */
    for(int j = 1; j < n; j++){
        for(int i = 0; i < j; i++){
            matrix_set(X, 0.0, i, j);
        }
    }

    return NULL;
}

void cholesky_N_threaded(matrix_t* X, int max_threads){
    pthread_t threads[max_threads];
    thread_cholesky_t data[max_threads];
    
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_t mutex2;
    pthread_cond_t cond2;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&cond2, NULL);

    int first_thread_finished = 0;
    int unblocked_after_first_finished = 0;
    int finished_threads = 0;
    int unblocked_after_all_finished = 0;

    int indices[max_threads+1];

    for(int i = 0; i < max_threads; i++){
        initialize_cholesky_thread_data(&data[i], X, i, &mutex, &mutex2, &cond, &cond2, &first_thread_finished, &unblocked_after_first_finished, &finished_threads, &unblocked_after_all_finished, max_threads, &indices);
        pthread_create(&threads[i], NULL, cholesky_thread, (void*) &data[i]);
    }

    for(int i = 0; i < max_threads; i++){
        pthread_join(threads[i], NULL);
    }

}

matrix_t* solve_cholesky_threaded(matrix_t* X, matrix_t* b, int max_threads){
    cholesky_N_threaded(X, max_threads);
    //cholesky_outer(X);
    fprintf(stderr, "Solved LLT!\n");

    matrix_t* y = forward_substitution(X, b, 0);
    fprintf(stderr, "Solved forward subsitution!\n");
    matrix_t* w = back_substitution(X, y, 1);
    fprintf(stderr, "Solved backward subsitution!\n");

    matrix_free(y);

    return w;
}

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

    //fprintf(stderr, "[Thread %d] (%d,%d)=%d (%d, %d)=%d\n", thread_id, from_row, until_row, until_row-from_row, from_col, until_col, until_col-from_col);
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
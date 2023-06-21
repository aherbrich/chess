#include "../include/matrix.h"

void simple_gram_matrix_test(){
    matrix_t* X = matrix_init(4,3);
    matrix_set(X, 1.0, 0, 0);
    matrix_set(X, 2.0, 0, 1);
    matrix_set(X, 3.0, 0, 2);
    matrix_set(X, 4.0, 1, 0);
    matrix_set(X, 5.0, 1, 1);
    matrix_set(X, 6.0, 1, 2);
    matrix_set(X, 7.0, 2, 0);
    matrix_set(X, 8.0, 2, 1);
    matrix_set(X, 9.0, 2, 2);
    matrix_set(X, 10.0, 3, 0);
    matrix_set(X, 11.0, 3, 1);
    matrix_set(X, 12.0, 3, 2);
    matrix_print(X);

    matrix_t* res = matrix_mult_gram_N_threaded(X, 7);
    matrix_print(res);

    matrix_free(X);
    matrix_free(res);
}

void extensive_gram_matrix_test(){
    struct timeval start;
    struct timeval end;
    double delta_in_ms;

    int n = 50000;
    int m = 2000;

    matrix_t* X = matrix_init(n,m);   
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            matrix_set(X, (double)rand()/(double)(RAND_MAX), i, j);
        }
    }

    gettimeofday(&start, 0);

    matrix_t* res = matrix_mult_gram_N_threaded(X, 10);
    printf("%f", matrix_read(res, 123,7));

    gettimeofday(&end, 0); 
    delta_in_ms = ((end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f);
    printf("\nTime: \t\t%fms\n", delta_in_ms);

    matrix_free(res);
    matrix_free(X);
}

void simple_cholesky_test(){
    matrix_t* X = matrix_init(3,4);
    matrix_set(X, 1.0, 0, 0);
    matrix_set(X, 1.0, 0, 1);
    matrix_set(X, 1.0, 0, 2);
    matrix_set(X, 1.0, 0, 3);
    matrix_set(X, 1.0, 1, 0);
    matrix_set(X, 2.0, 1, 1);
    matrix_set(X, 4.0, 1, 2);
    matrix_set(X, 8.0, 1, 3);
    matrix_set(X, 1.0, 2, 0);
    matrix_set(X, 3.0, 2, 1);
    matrix_set(X, 9.0, 2, 2);
    matrix_set(X, 27.0, 2, 3);
    matrix_print(X);

    matrix_t* XT = matrix_transpose(X);
    matrix_print(XT);

    matrix_t* y = matrix_init(3,1);
    matrix_set(y, 1.0, 0, 0);
    matrix_set(y, 4.0, 1, 0);
    matrix_set(y, 0.0, 2, 0);
    matrix_print(y);


    matrix_t* XTX = matrix_mult_gram_N_threaded(X, 11);
    matrix_regularize(XTX, 0.001);
    matrix_t* b = matrix_mult(XT, y);

    matrix_t* res = solve_cholesky(XTX, b);
    matrix_print(res);

    matrix_free(X);
    matrix_free(XT);
    matrix_free(y);
    matrix_free(b);
    matrix_free(res);
}


int main(){
    extensive_gram_matrix_test();
    simple_gram_matrix_test();
    simple_cholesky_test();
}
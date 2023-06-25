#include "../include/matrix.h"
#include "../include/chess.h"

board_t* OLDSTATE[2048];
uint64_t HISTORY_HASHES[2048];

int nodes_searched = 0;
int hash_used = 0;
int hash_bounds_adjusted = 0;
int pv_node_hit= 0;


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

    //matrix_t* res = matrix_mult_gram_N_threaded(X, 7);
    matrix_t* res = matrix_mult_gram(X);
    matrix_print(res);

    matrix_free(X);
    matrix_free(res);
}

void extensive_gram_matrix_test(){
    struct timeval start;
    struct timeval end;
    double delta_in_ms;

    int n = 25000;
    int m = 1000;

    matrix_t* X = matrix_init(n,m);   
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            matrix_set(X, (double)rand()/(double)(RAND_MAX), i, j);
        }
    }

    gettimeofday(&start, 0);

    //matrix_t* res = matrix_mult_gram_N_threaded(X, 1);
    matrix_t* res = matrix_mult_gram(X);
    fprintf(stderr, "%f", matrix_read(res, 123,7));

    gettimeofday(&end, 0); 
    delta_in_ms = ((end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f);
    fprintf(stderr, "\nTime: \t\t%fms\n", delta_in_ms);

    matrix_free(res);
    matrix_free(X);
}

void simple_cholesky_test(){
    matrix_t* X = matrix_init(5,6);
    matrix_set(X, 1.0, 0, 0);
    matrix_set(X, 1.0, 0, 1);
    matrix_set(X, 1.0, 0, 2);
    matrix_set(X, 1.0, 0, 3);
    matrix_set(X, 1.0, 0, 4);
    matrix_set(X, 1.0, 0, 5);

    matrix_set(X, 1.0, 1, 0);
    matrix_set(X, 2.0, 1, 1);
    matrix_set(X, 4.0, 1, 2);
    matrix_set(X, 8.0, 1, 3);
    matrix_set(X, 16.0, 1, 4);
    matrix_set(X, 32.0, 1, 5);

    matrix_set(X, 1.0, 2, 0);
    matrix_set(X, 3.0, 2, 1);
    matrix_set(X, 9.0, 2, 2);
    matrix_set(X, 27.0, 2, 3);
    matrix_set(X, 81.0, 2, 4);
    matrix_set(X, 243.0, 2, 5);

    matrix_set(X, 1.0, 3, 0);
    matrix_set(X, 4.0, 3, 1);
    matrix_set(X, 16.0, 3, 2);
    matrix_set(X, 64.0, 3, 3);
    matrix_set(X, 256.0, 3, 4);
    matrix_set(X, 1024.0, 3, 5);

    matrix_set(X, 1.0, 4, 0);
    matrix_set(X, 5.0, 4, 1);
    matrix_set(X, 25.0, 4, 2);
    matrix_set(X, 125.0, 4, 3);
    matrix_set(X, 625.0, 4, 4);
    matrix_set(X, 3125.0, 4, 5);

    matrix_print(X);

    matrix_t* XT = matrix_transpose(X);
    matrix_print(XT);

    matrix_t* y = matrix_init(5,1);
    matrix_set(y, 1.0, 0, 0);
    matrix_set(y, 4.0, 1, 0);
    matrix_set(y, 0.0, 2, 0);
    matrix_set(y, 2.0, 3, 0);
    matrix_set(y, -1.0, 4, 0);
    matrix_print(y);


    //matrix_t* XTX = matrix_mult_gram_N_threaded(X, 11);
    matrix_t* XTX = matrix_mult_gram(X);
    matrix_regularize(XTX, 0.001);
    matrix_print(XTX);
    matrix_t* b = matrix_mult(XT, y);


    matrix_t* res2 = solve_cholesky(XTX, b);
    matrix_print(res2);

    matrix_t* res = solve_cholesky_threaded(XTX, b, 3);
    matrix_print(res);

    matrix_free(X);
    matrix_free(XT);
    matrix_free(y);
    matrix_free(XTX);
    matrix_free(b);
    matrix_free(res);
}

void extensive_cholesky_test(){
    struct timeval start;
    struct timeval end;
    double delta_in_ms;

    int m = 5000;
    int n = m*10;
    

    matrix_t* X = matrix_init(n,m);   
    matrix_t* y = matrix_init(n, 1);

    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            matrix_set(X, (double)rand()/(double)(RAND_MAX), i, j);
        }
    }

    for(int i = 0; i < n; i++){
        matrix_set(y, (double)rand()/(double)(RAND_MAX), i, 0);
    }


    matrix_t* XTX = matrix_mult_gram_N_threaded(X, 32);
    matrix_regularize(XTX, 0.001);


    fprintf(stderr, "Solved XTX!\n");

    matrix_t* b = matrix_mult_first_arg_transposed(X, y);

    gettimeofday(&start, 0);
    matrix_t* res = solve_cholesky(XTX, b);
    gettimeofday(&end, 0); 
    delta_in_ms = ((end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f);
    fprintf(stderr, "\nTime: \t\t%fms\n", delta_in_ms);


    gettimeofday(&start, 0);
    matrix_t* res2 = solve_cholesky_threaded(XTX, b, 32); 
    gettimeofday(&end, 0); 
    delta_in_ms = ((end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f);
    fprintf(stderr, "\nTime: \t\t%fms\n", delta_in_ms);

    fprintf(stderr, "%f %f\n", matrix_read(res, 3, 0),matrix_read(res2, 3, 0));



    matrix_free(X);
    matrix_free(y);
    matrix_free(XTX);
    matrix_free(b);
    matrix_free(res);
    matrix_free(res2);
}


int main(){
    // extensive_gram_matrix_test();
    // simple_gram_matrix_test();
    //simple_cholesky_test();
    extensive_cholesky_test();
    printf("Ran all tests!\n");
}
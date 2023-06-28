#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"
#include "../include/database.h"
#include "../include/san.h"
#include "../include/parse.h"
#include "../include/features.h"
#include "../include/linalg.h"
#include <string.h>

board_t* OLDSTATE[2048];
uint64_t HISTORY_HASHES[2048];


int nodes_searched = 0;
int hash_used = 0;
int hash_bounds_adjusted = 0;
int pv_node_hit= 0;

/*  Playes all games in given chess game list and    
    stores all chess positions and corresponding winrates */
void load_games_into_database(chessgame_t** chessgames, int nr_of_games){
     // play games
    for(int i = 0; i < nr_of_games; i++){
        if((i%1000) == 0){
            fprintf(stderr, ".");
        }
        chessgame_t* chessgame = chessgames[i];
        // (0) intialize board 
        board_t *board = init_board();
        load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        // (1) read move 
        char* token = strtok(chessgame->movelist, " ");

        do{
            // (2) parse move
            move_t *move = str_to_move(board, token);

            // (3) play move
            if (move){
                do_move(board, move);
                update_database_entry(board, chessgame->winner);
                free_move(move);
            }
            else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n",Color_PURPLE, token, Color_END);
                exit(-1);
            }
        // (4) repeat until all moves played
        } while((token = strtok(NULL, " ")));

        free_board(board);
    }

    // free chess games
    for(int i = 0; i < nr_of_games; i++){
        free(chessgames[i]->movelist);
    }
    free(chessgames);
}


int main(){
    // parse chess game file
    int nr_of_games = count_number_of_games();
    chessgame_t **chessgames = parse_chessgames_file(nr_of_games);

    // initialize chess engine
    initialize_chess_engine_only_necessary();
    initialize_zobrist_table();
    initialize_database();

    load_games_into_database(chessgames, nr_of_games);

    // number of different boards = 1739062

    int m = 23040;
    int n = m*10;
    

    matrix_t* X = matrix_init(n, m);
    matrix_t* y = matrix_init(n, 1);

    int count = 0;
    for (int i = 0; i < DATABASESIZE && count < n; i++) {
        databaseentry_t* tmp = database[i];
        
        while(tmp && count < n){
            double expected_result = (double) (tmp->white_won - tmp->black_won)/tmp->seen;
            matrix_set(y, expected_result, count, 0);
            calculate_feautures(tmp->board, X, count);   
            count++;           
            tmp = tmp->next;
        } 
    }

    fprintf(stderr, "\nUnique positions:%d\n", count);

    matrix_t* XTX = matrix_mult_gram_N_threaded(X, 32);
    matrix_regularize(XTX, 0.001);
    fprintf(stderr, "Solved XTX!\n");

    matrix_t* b = matrix_mult_first_arg_transposed(X, y);

    matrix_free(X);
    matrix_free(y);

    fprintf(stderr, "Solving with cholesky!\n");
    matrix_t* w = solve_cholesky(XTX, b);

    for(int test = 0; test < 12; test++){
        for(int time = 0; time < 30; time++){
            for(int row = 7; row >= 0; row--){
                for(int col = 0; col < 8; col++){
                    printf("%.2f  ", matrix_read(w, (row*8+col)+(time*64)+(test*1920), 0));
                }
                printf("\n");
            }
            printf("\n\n");
        }
        printf("\n=============================\n");
    }
}

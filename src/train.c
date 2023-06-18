#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"
#include "../include/database.h"
#include "../include/san.h"
#include "../include/parse.h"
#include <string.h>

board_t* OLDSTATE[2048];
uint64_t HISTORY_HASHES[2048];

databaseentry_t** database;

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

    board_t *board = init_board();
    load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    probe_database_entry(board);
}

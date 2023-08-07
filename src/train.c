#include <string.h>
#include <stdio.h>

#include "../include/engine.h"
#include "../include/database.h"
#include "../include/parse.h"


/*  Playes all games in given chess game list and
    stores all chess positions and corresponding winrates */
void load_games_into_database(chessgame_t** chessgames, int nr_of_games) {
    /* play games */
    for (int i = 0; i < nr_of_games; i++) {
        if ((i % 1000) == 0) {
            fprintf(stderr, ".");
        }
        chessgame_t* chessgame = chessgames[i];
        /* (0) intialize board */
        board_t* board = init_board();
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        /* (1) read move */
        char* token = strtok(chessgame->movelist, " ");

        do {
            /* (2) parse move */
            move_t* move = str_to_move(board, token);

            /* (3) play move */
            if (move) {
                do_move(board, move);
                update_database_entry(board, chessgame->winner);
                free_move(move);
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token,
                        Color_END);
                exit(-1);
            }
            /* (4) repeat until all moves played */
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    /* free chess games */
    for (int i = 0; i < nr_of_games; i++) {
        free(chessgames[i]->movelist);
    }
    free(chessgames);
}


/*  Playes all games in given chess game list and
    prints all moves played */
void load_moves_into_database(chessgame_t** chessgames, int nr_of_games) {
    /* play games */
    for (int i = 0; i < nr_of_games; i++) {
        chessgame_t* chessgame = chessgames[i];
        /* (0) intialize board */
        board_t* board = init_board();
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        /* (1) read move */
        char* token = strtok(chessgame->movelist, " ");

        do {
            /* (2) parse move */
            move_t* move = str_to_move(board, token);

            /* (3) play move */
            if (move) {

                /* print made move */
                print_move_ranking(board, move);
                printf("{");
                /* find and print all other moves */
                maxpq_t movelst;
                initialize_maxpq(&movelst);
                generate_moves(board, &movelst);
                move_t* other_move;
                while ((other_move = pop_max(&movelst)) != NULL) {
                    if(is_same_move(move, other_move)){
                        free_move(other_move);
                        continue;
                    }
                    print_move_ranking(board, other_move);
                    free_move(other_move);
                }
                printf("}");
                /* execute made move */
                do_move(board, move);
                free_move(move);
                printf("\n");
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token,
                        Color_END);
                exit(-1);
            }
            /* (4) repeat until all moves played */
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    /* free chess games */
    for (int i = 0; i < nr_of_games; i++) {
        free(chessgames[i]->movelist);
    }
    free(chessgames);
}

int main() {
    /* parse chess game file */
    int nr_of_games = count_number_of_games();
    chessgame_t** chessgames = parse_chessgames_file(nr_of_games);

    /* initialize chess engine */
    initialize_chess_engine_necessary();
    initialize_zobrist_table();
    initialize_database();

    load_moves_into_database(chessgames, nr_of_games);
    /* number of different boards = 1739062 */
}



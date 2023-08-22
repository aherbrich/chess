#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/engine-core/engine.h"
#include "include/engine-core/init.h"
#include "include/parse/parse.h"
#include "include/train-eval/database.h"

/*  Plays all games in given chess game list and stores all chess positions and corresponding win-rates */
void load_games_into_database(chess_game_t** chess_games, int nr_of_games) {
    /* play games */
    for (int i = 0; i < nr_of_games; i++) {
        if ((i % 1000) == 0) {
            fprintf(stderr, ".");
        }
        chess_game_t* chess_game = chess_games[i];
        /* (0) intialize board */
        board_t* board = init_board();
        load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        /* (1) read move */
        char* token = strtok(chess_game->move_list, " ");

        do {
            /* (2) parse move */
            move_t* move = str_to_move(board, token);

            /* (3) play move */
            if (move) {
                do_move(board, move);
                update_database_entry(board, chess_game->winner);
                free_move(move);
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token, Color_END);
                exit(-1);
            }
            /* (4) repeat until all moves played */
        } while ((token = strtok(NULL, " ")));

        free_board(board);
    }

    return;
}

int main() {
    /* parse chess game file */
    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn");
    chess_games_t chess_games = load_chess_games(file_name);

    /* initialize chess engine */
    initialize_attack_boards();
    initialize_helper_boards();
    initialize_zobrist_table();
    initialize_database();

    /* simulate all games and load current-board-to-win-percentage related data into database */
    load_games_into_database(chess_games.games, chess_games.no_games);

    return 0;
    /* number of different boards = 1739062 */
}

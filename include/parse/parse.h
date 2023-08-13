#ifndef __PARSE_H__
#define __PARSE_H__

#include "include/engine-core/types.h"

typedef enum _game_result_t {
    BLACK_WIN = -1, DRAW = 0, WHITE_WIN = 1
} game_result_t;

typedef struct _chess_game_t {
    char* move_list;
    int winner;
} chess_game_t;

//  FUNCTIONS
int count_number_of_games();
int count_moves_made(chess_game_t** chess_games, int nr_of_games);
chess_game_t** parse_chess_games_file(int nr_of_games);

move_t* str_to_move(board_t* board, char* token);

#endif
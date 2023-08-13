#ifndef __PARSE_H__
#define __PARSE_H__

#include "include/engine-core/types.h"

typedef enum _game_result_t {
    BLACK_WIN = -1,
    DRAW = 0,
    WHITE_WIN = 1
} game_result_t;

typedef struct _chess_game_t {
    char* move_list;
    int winner;
} chess_game_t;

typedef struct _chess_games_t {
    int no_games;          // number of games
    chess_game_t** games;  // array of chess games
} chess_games_t;

/* loads a set of chess games from a text file */
chess_games_t load_chess_games(const char* file_name);
/* count total numbers of moves made */
int count_moves_made(chess_games_t chess_games);
/* frees the memory for a set of chess games */
void delete_chess_games(chess_games_t chess_games);

move_t* str_to_move(board_t* board, char* token);

#endif
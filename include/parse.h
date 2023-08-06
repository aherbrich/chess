#ifndef __PARSE_H__
#define __PARSE_H__

typedef enum _gameresult_t {
    WINBLACK = -1, DRAW = 0, WINWHITE = 1
} gameresult_t;

typedef struct _chessgame_t {
    char* movelist;
    int winner;
} chessgame_t;

//////////////////////////////////////////////////////////////
//  FUNCTIONS
int count_number_of_games();
int count_moves_made(chessgame_t** chessgames, int nr_of_games);
chessgame_t** parse_chessgames_file(int nr_of_games);

#endif
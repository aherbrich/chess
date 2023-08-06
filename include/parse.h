#ifndef __PARSE_H__
#define __PARSE_H__

#include "../include/chess.h"

//////////////////////////////////////////////////////////////
//  FUNCTIONS
int count_number_of_games();
int count_moves_made(chessgame_t** chessgames, int nr_of_games);
chessgame_t** parse_chessgames_file(int nr_of_games);

#endif
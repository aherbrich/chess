#ifndef __PARSE_H__
#define __PARSE_H__

#include "../include/chess.h"

/* Counts the number of games in a PGN chess game file */
int count_number_of_games();
/* Parses PGN chess game file */
/* i.e stores games in accesible way in memory */
chessgame_t** parse_chessgames_file(int nr_of_games);

#endif
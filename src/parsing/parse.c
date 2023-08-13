#include "include/parse/parse.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Counts the number of games in a PGN chess game file */
int count_number_of_games() {
    int game_count = 0;
    int buffersize = 8192;
    char buffer[buffersize];

    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn");

    /* open file */
    FILE *fp = fopen(file_name, "r");

    /* read line by line */
    while (fgets(buffer, buffersize, fp) != NULL) {
        /* if we read a newline */
        if (!strcmp(buffer, "\r\n")) {
            /* the next line SHOULD be the line of moves */
            fgets(buffer, buffersize, fp);

            /* check if the line is ACTUALLY the line of moves */
            if (strstr(buffer, "1.")) {
                game_count++;
            }

            /* skip the next newline */
            fgets(buffer, buffersize, fp);

            /* repeat... */
        }
    }
    fclose(fp);

    return game_count;
}

int count_moves_made(chess_game_t **chess_games, int nr_of_games) {
    int total_count = 0;
    for (int i = 0; i < nr_of_games; i++) {
        chess_game_t *chess_game = chess_games[i];
        char *token = strtok(chess_game->move_list, " ");
        do {
            total_count++;
        } while ((token = strtok(NULL, " ")));
    }

    return total_count;
}

/* Parses PGN chess game file, i.e stores games in accessible way in memory */
chess_game_t **parse_chess_games_file(int nr_of_games) {
    /* allocate memory for chess game data structures and initialize */
    chess_game_t **chess_games =
        (chess_game_t **)malloc(sizeof(chess_game_t *) * nr_of_games);
    for (int i = 0; i < nr_of_games; i++) {
        chess_games[i] = NULL;
    }

    char file_name[PATH_MAX];
    getcwd(file_name, PATH_MAX);
    strcat(file_name, "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn");

    /* open file */
    FILE *fp = fopen(file_name, "r");

    int buffersize = 8192;
    char buffer[buffersize];
    char parsedbuffer[buffersize];
    int game_idx = 0;
    int winner = 0;

    /* read line by line */
    while (fgets(buffer, buffersize, fp) != NULL) {
        /* if we read the line with the result of a game, remember the result */
        if (strstr(buffer, "Result \"1-0\"")) {
            winner = WHITE_WIN;
        } else if (strstr(buffer, "Result \"0-1\"")) {
            winner = BLACK_WIN;
        } else if (strstr(buffer, "Result \"1/2-1/2\"")) {
            winner = DRAW;
        }

        /* if we read a newline */
        if (!strcmp(buffer, "\r\n")) {
            /* the next line SHOULD be the line of moves */
            fgets(buffer, buffersize, fp);

            /* check if the line is ACTUALLY the line of moves */
            if (strstr(buffer, "1.")) {
                /* initialize buffer into which we parse the move string */
                for (int i = 0; i < buffersize; i++) {
                    parsedbuffer[i] = '\0';
                }

                /* read moves */
                char *ptr = strtok(buffer, " ");
                int move_counter = 0;
                int idx = 0;
                while ((ptr = strtok(NULL, " ")) && ptr[0] != '{') {
                    /* every two tokens, skip a token (since that token is a
                     * move number and not a move) */
                    if (move_counter == 2) {
                        move_counter = 0;
                        continue;
                    }
                    move_counter++;

                    /* copy move into buffer */
                    strcpy(parsedbuffer + idx, ptr);
                    idx += strlen(ptr);
                    parsedbuffer[idx++] = ' ';
                }
                parsedbuffer[idx - 1] = '\0';

                /* store data in chess game structure */
                chess_game_t *chess_game =
                    (chess_game_t *)malloc(sizeof(chess_game_t));
                chess_game->move_list = (char *)malloc(strlen(parsedbuffer) + 1);
                strcpy(chess_game->move_list, parsedbuffer);
                chess_game->winner = winner;

                /* add chess game into list of chess games */
                chess_games[game_idx] = chess_game;
                game_idx++;
            }

            /* skip the next newline */
            fgets(buffer, buffersize, fp);
        }
    }

    fclose(fp);
    fprintf(stderr, "Nr. of games:\t%d\n", nr_of_games);
    return chess_games;
}
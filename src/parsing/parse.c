#include "include/parse/parse.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* counts the number of games in a PGN chess game file */
int count_number_of_games(const char* file_name) {
    int game_count = 0;
    int buffer_size = 8192;
    char buffer[buffer_size];

    /* open file */
    FILE *fp = fopen(file_name, "r");

    /* read line by line */
    while (fgets(buffer, buffer_size, fp) != NULL) {
        /* if we read a newline */
        if (!strcmp(buffer, "\r\n")) {
            /* the next line SHOULD be the line of moves */
            fgets(buffer, buffer_size, fp);

            /* check if the line is ACTUALLY the line of moves */
            if (strstr(buffer, "1.")) {
                game_count++;
            }

            /* skip the next newline */
            fgets(buffer, buffer_size, fp);

            /* repeat... */
        }
    }
    fclose(fp);

    return game_count;
}

/* Parses PGN chess game file, i.e stores games in accessible way in memory */
chess_game_t **parse_chess_games_file(const char* file_name, int no_games) {
    /* allocate memory for chess game data structures and initialize */
    chess_game_t **chess_games =
        (chess_game_t **)malloc(sizeof(chess_game_t *) * no_games);
    for (int i = 0; i < no_games; i++) {
        chess_games[i] = NULL;
    }

    /* open file */
    FILE *fp = fopen(file_name, "r");

    int buffer_size = 8192;
    char buffer[buffer_size];
    char parsed_buffer[buffer_size];
    int game_idx = 0;
    int winner = 0;

    /* read line by line */
    while (fgets(buffer, buffer_size, fp) != NULL) {
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
            fgets(buffer, buffer_size, fp);

            /* check if the line is ACTUALLY the line of moves */
            if (strstr(buffer, "1.")) {
                /* initialize buffer into which we parse the move string */
                for (int i = 0; i < buffer_size; i++) {
                    parsed_buffer[i] = '\0';
                }

                /* read moves */
                char *ptr = strtok(buffer, " ");
                int move_counter = 0;
                int idx = 0;
                while ((ptr = strtok(NULL, " ")) && ptr[0] != '{') {
                    /* every two tokens, skip a token (since that token is a move number and not a move) */
                    if (move_counter == 2) {
                        move_counter = 0;
                        continue;
                    }
                    move_counter++;

                    /* copy move into buffer */
                    strcpy(parsed_buffer + idx, ptr);
                    idx += strlen(ptr);
                    parsed_buffer[idx++] = ' ';
                }
                parsed_buffer[idx - 1] = '\0';

                /* store data in chess game structure */
                chess_game_t *chess_game = (chess_game_t *)malloc(sizeof(chess_game_t));
                chess_game->move_list = (char *)malloc(strlen(parsed_buffer) + 1);
                strcpy(chess_game->move_list, parsed_buffer);
                chess_game->winner = winner;

                /* add chess game into list of chess games */
                chess_games[game_idx] = chess_game;
                game_idx++;
            }

            /* skip the next newline */
            fgets(buffer, buffer_size, fp);
        }
    }

    fclose(fp);
    fprintf(stderr, "Nr. of games:\t%d\n", no_games);
    return chess_games;
}

/* loads a set of chess games from a text file */
chess_games_t load_chess_games(const char* file_name) {
    int no_games = count_number_of_games(file_name);
    chess_games_t chess_games = {
        .no_games = no_games,
        .games = parse_chess_games_file(file_name, no_games)};
    return chess_games;
}

/* count total numbers of moves made */
int count_moves_made(chess_games_t chess_games) {
    int total_count = 0;
    for (int i = 0; i < chess_games.no_games; i++) {
        chess_game_t *chess_game = chess_games.games[i];
        char *token = strtok(chess_game->move_list, " ");
        do {
            total_count++;
        } while ((token = strtok(NULL, " ")));
    }

    return total_count;
}

/* frees the memory for a set of chess games */
void delete_chess_games(chess_games_t chess_games) {
    for (int i = 0; i < chess_games.no_games; i++) {
        free(chess_games.games[i]->move_list);
        free(chess_games.games[i]);
    }
    free(chess_games.games);
    return;
}
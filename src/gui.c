#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/engine-core/engine.h"

int MAX_LEN = 32768;

pthread_t game_thread = 0;

/* Converts a string to a move index */
idx_t gui_str_to_idx(char *ptr, int len) {
    idx_t idx = 0;
    for (int i = 0; i < len; i++) {
        switch (*ptr) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
                idx = idx + ((*ptr) - 'a');
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
                idx = idx + ((*ptr) - 'A');
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                idx = idx + (8 * (*ptr - '1'));
                break;
        }
        ptr++;
    }
    return (idx);
}

/* Converts a string to a move */
move_t *gui_str_to_move(board_t *board, char *move_str) {
    /* extracts the from and to position of the move */
    idx_t from = gui_str_to_idx(move_str, 2);
    idx_t to = gui_str_to_idx(&(move_str[2]), 2);

    /* generate all possible moves in the current position */
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    /* find the move in the movelist that is described by the move string */
    for (int i = 1; i < (&movelst)->nr_elem + 1; i++) {
        move_t *move = (&movelst)->array[i];
        /* if the move has matching from and to squares */
        if (move->from == from && move->to == to) {
            /* if promotion move */
            if (move->flags & 0b1000 && strlen(move_str) == 5) {
                if ((move_str[4] == 'q' || move_str[4] == 'Q') &&
                    (move->flags == QPROM || move->flags == QCPROM)) {
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                } else if ((move_str[4] == 'r' || move_str[4] == 'R') &&
                           (move->flags == RPROM || move->flags == RCPROM)) {
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                } else if ((move_str[4] == 'b' || move_str[4] == 'B') &&
                           (move->flags == BPROM || move->flags == BCPROM)) {
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                } else if ((move_str[4] == 'n' || move_str[4] == 'N') &&
                           (move->flags == KPROM || move->flags == KCPROM)) {
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
            }
            /* if not a promotion move */
            else if (!(move->flags & 0b1000) && strlen(move_str) == 4) {
                move_t *copy = copy_move(move);
                free_pq(&movelst);
                return copy;
            }
        }
    }

    free_pq(&movelst);

    return NULL;
}

/* Parses the go command */
searchdata_t *parse_go_command(char *token, board_t *board) {
    const char delim[] = " \n\t";
    searchdata_t *data = init_search_data(board);

    /* if no time is specified, just run infinite */
    data->run_infinite = 1;
    data->max_depth = 100;

    while ((token = strtok(NULL, delim))) {
        if (!strcmp(token, "wtime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for white%s\n",
                        Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->wtime = atoi(token);
                data->run_infinite = 0;
            }
        } else if (!strcmp(token, "btime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for black%s\n",
                        Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->btime = atoi(token);
                data->run_infinite = 0;
            }
        } else if (!strcmp(token, "winc")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for white increment%s\n",
                        Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->winc = atoi(token);
                data->run_infinite = 0;
            }
        } else if (!strcmp(token, "binc")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for black increment%s\n",
                        Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->binc = atoi(token);
                data->run_infinite = 0;
            }
        } else if (!strcmp(token, "movetime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing movetime%s\n", Color_PURPLE,
                        Color_END);
                exit(-1);
            } else {
                data->max_time = atoi(token);
                data->run_infinite = 0;
            }
        } else if (!strcmp(token, "depth")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing depth%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->max_depth = atoi(token);
            }
        } else if (!strcmp(token, "infinite")) {
            /* nothing to do */
            break;
        } else {
            fprintf(stderr, "%sincorrect command: %s%s\n", Color_PURPLE, token,
                    Color_END);
        }
    }

    /* Block this thread and wait for initiation of the new search until running
     * search has finished */
    while (game_thread) {
        fprintf(stderr, "%sSearch underway ... sleeping for 1 second%s\n",
                Color_PURPLE, Color_END);
        sleep(1);
    }

    return data;
}

/* Parses position command */
void parse_position_command(char *token, board_t *board) {
    const char delim[] = " \n\t";
    char fen[MAX_LEN];

    token = strtok(NULL, delim);

    /* handle the parsing of position given either by fen or startpos */
    /* handle startpos */
    if (!strcmp(token, "startpos")) {
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        token = strtok(NULL, delim);
        if (token && strcmp(token, "moves")) {
            fprintf(
                stderr,
                "%sIncorrect syntax in the 'position startpos' command: %s%s\n",
                Color_PURPLE, token, Color_END);
            return;
        }
    }
    /* handle fen string */
    else if (!strcmp(token, "fen")) {
        int fen_idx = 0;
        token = strtok(NULL, delim);
        while (token && strcmp(token, "moves")) {
            int len = strlen(token);
            strcpy(&(fen[fen_idx]), token);
            fen_idx += len;
            fen[fen_idx++] = ' ';
            token = strtok(NULL, delim);
        }
        fen[fen_idx] = '\0';
        load_by_FEN(board, fen);
    } else {
        fprintf(stderr, "%sIncorrect syntax in the 'position' command: %s%s\n",
                Color_PURPLE, token, Color_END);
    }

    /* finally, read all the moves, if there are any */
    if (token) {
        while ((token = strtok(NULL, delim))) {
            move_t *move = gui_str_to_move(board, token);
            if (move) {
                do_move(board, move);
                free_move(move);
            } else {
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token,
                        Color_END);
                exit(-1);
            }
        }
    }

    return;
}

/* Starts the search intitiated by user/gui */
void *start_search(void *args) {
    searchdata_t *data = (searchdata_t *)args;

    /* clear hash table form last search*/
    clear_hashtable();
    /* start iterative search */
    search(data);

    /* signal communication thread that search is finished */
    game_thread = 0;

    free_search_data(data);
    pthread_exit(NULL);
}

/* runs the main loop of the the UCI communication interface */
void *main_interface_loop(void *args) {
    board_t *board = (board_t *)args;
    searchdata_t *searchdata = NULL;

    /* remove buffering from stdin and stdout */
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    /* print chess engine info */
    printf("id name Engine v2.0\nid author Alexander Herbrich\n\n");
    printf("uciok\n");

    while (1) {
        char buffer[MAX_LEN];
        char *token;
        const char delim[] = " \n\t";

        /* read a whole line from input */
        fgets(buffer, MAX_LEN - 1, stdin);

        /* get the first token */
        token = strtok(buffer, delim);

        /* if no token read, continue reading */
        if (!token) continue;

        /* else if token read
        START parsing */

        /* UCI command */
        if (!strcmp(token, "uci")) {
            printf("id name Engine v2.0\nid author Alexander Herbrich\n\n");
            printf("uciok\n");
        }
        /* ISREADY command */
        else if (!strcmp(token, "isready")) {
            printf("readyok\n");
        }
        /* GO command */
        else if (!strcmp(token, "go")) {
            searchdata = parse_go_command(token, board);
            pthread_create(&game_thread, NULL, start_search,
                           (void *)searchdata);
        }
        /* NEWGAME command */
        else if (!strcmp(token, "ucinewgame")) {
            clear_board(board);
            clear_hashtable();
        }
        /* POSITION command */
        else if (!strcmp(token, "position")) {
            parse_position_command(token, board);
        }
        /* PRINT */
        else if (!strcmp(token, "bugprint")) {
            print_board(board);
        }
        /* STOP command */
        else if (!strcmp(token, "stop")) {
            if (searchdata) {
                searchdata->stop = 1;
            }
        }
        /* QUIT command */
        else if (!strcmp(token, "quit")) {
            break;
        }
    }

    return (NULL);
}

/* MAIN ENTRY POINT */
int main() {
    /* Initialize all necessary structures */
    board_t *board = init_board();
    initialize_chess_engine_necessary();
    initialize_zobrist_table();
    initialize_hashtable();

    /* start chess engine */
    main_interface_loop(board);

    /* free the board */
    free_board(board);
}
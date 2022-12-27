#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/book.h"
#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"

#define MAX_LEN 1024

char *STARTING_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

////////////////////////////////////////////////////////////////
// Option management

/* type definition of the value type of an option */
typedef enum {
    OPT_CHECK,  /* boolean option value */
    OPT_SPIN,   /* spin option value */
    OPT_BUTTON, /* button (without value) */
    OPT_STRING  /* string option value */
} option_value_type_t;

/* type definition of the value of an option */
typedef union _option_value_t {
    int bool_value; /* Boolean value */
    struct {
        int default_v;  /* default value */
        int min_v;      /* minimum value */
        int max_v;      /* maximum value */
    } spin_value;       /* Spin value */
    char *string_value; /* String value */
} option_value_t;

/* linked list of chess engine options */
typedef struct _options_t {
    char *key;                              /* key of the option */
    option_value_type_t value_type;         /* type of the value */
    option_value_t value;                   /* (actual) value of the option value */
    void (*on_change)(struct _options_t *); /* call-back when the value gets changed */
    struct _options_t *next;                /* pointer to next option */
} options_t;

/* adds an option to the linked list and returns new head */
options_t *add_option(options_t *head, const char *key, option_value_type_t value_type, option_value_t value, void (*on_change)(struct _options_t *)) {
    options_t *new_head = (options_t *)malloc(sizeof(options_t));

    /* make a deep copy of the key string */
    new_head->key = (char *)malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(new_head->key, key);

    new_head->value_type = value_type;
    new_head->value = value;
    new_head->on_change = on_change;
    if (value_type == OPT_STRING) {
        /* make a deep copy of the value string (if necessary) */
        new_head->value.string_value = (char *)malloc(sizeof(char) * (strlen(value.string_value) + 1));
        strcpy(new_head->value.string_value, value.string_value);
    }

    /* promote the new entry to the new head */
    new_head->next = head;
    return (new_head);
}

/* gets the value for a given key in the options list and returns the value via reference;
   returns 1, if successful or 0 if the kye cannot be found */
int get_option_value(options_t *head, const char *key, option_value_type_t *value_type, option_value_t *value) {
    while (head) {
        if (!strcmp(head->key, key)) {
            *value_type = head->value_type;
            *value = head->value;
            return (1);
        }

        head = head->next;
    }
    return (0);
}

/* sets the value for a given key in the options list and return 1, if successful or 0 if the kye cannot be found */
int set_options_value(options_t *head, const char *key, option_value_t value) {
    while (head) {
        /* note that we make a deep copy of the string, if this is the actual value */
        if (!strcmp(head->key, key)) {
            if (head->value_type == OPT_STRING) {
                free(head->value.string_value);
                head->value.string_value = (char *)malloc(sizeof(char) * (strlen(value.string_value) + 1));
                strcpy(head->value.string_value, value.string_value);
            } else {
                head->value = value;
            }

            /* invoke the call-back, if the call-back function is set */
            if (head->on_change)
                head->on_change(head);
            return (1);
        }

        head = head->next;
    }
    return (0);
}

/* print the options list on the screen */
void print_options(options_t *head) {
    while (head) {
        printf("option name %s ", head->key);
        switch (head->value_type) {
            case OPT_BUTTON:
                printf("type button");
                break;
            case OPT_CHECK:
                printf("type check default %s", (head->value.bool_value) ? "true" : "false");
                break;
            case OPT_SPIN:
                printf("type spin default %d min %d max %d", head->value.spin_value.default_v, head->value.spin_value.min_v, head->value.spin_value.max_v);
                break;
            case OPT_STRING:
                printf("type string default %s", head->value.string_value);
                break;
            default:
                fprintf(stderr, "%sInternal error!\n%s", Color_PURPLE, Color_END);
                exit(-1);
        }
        printf("\n");

        head = head->next;
    }

    return;
}

/* frees the memory of an options list */
void free_options(options_t *head) {
    while (head) {
        options_t *tmp = head->next;
        free(head->key);
        if (head->value_type == OPT_STRING)
            free(head->value.string_value);
        free(head);
        head = tmp;
    }

    return;
}

/* handles the clear hash table */
void on_clear_hashtable(options_t *opt) {
    fprintf(stderr, "%sClear Hash Table called%s\n", Color_PURPLE, Color_END);
    clear_hashtable();
    return;

    // This is only here to avoid a compiler warning
    opt++;
}

// /* initializes the options */
options_t *init_options(void) {
    options_t *head = NULL;
    option_value_t debug_log_value = {.string_value = ""};
    option_value_t thread_value = {.spin_value = {.default_v = 1, .min_v = 1, .max_v = 1}};
    option_value_t hash_value = {.spin_value = {.default_v = 16, .min_v = 1, .max_v = 67108864}};
    option_value_t clear_hash_value = {.bool_value = 0};
    option_value_t ponder_value = {.bool_value = 0};
    option_value_t multi_pv_value = {.spin_value = {.default_v = 1, .min_v = 1, .max_v = 1}};
    option_value_t skill_value = {.spin_value = {.default_v = 20, .min_v = 0, .max_v = 20}};
    option_value_t move_overhead_value = {.spin_value = {.default_v = 10, .min_v = 0, .max_v = 5000}};
    option_value_t nodes_time_value = {.spin_value = {.default_v = 0, .min_v = 0, .max_v = 10000}};
    option_value_t uci_chess960_value = {.bool_value = 0};
    option_value_t uci_analyze_mode_value = {.bool_value = 0};
    option_value_t uci_limit_strength_value = {.bool_value = 0};
    option_value_t uci_elo_value = {.spin_value = {.default_v = 1350, .min_v = 1350, .max_v = 1500}};
    option_value_t uci_show_wdl_value = {.bool_value = 0};

    head = add_option(head, "Debug Log File", OPT_STRING, debug_log_value, NULL);
    head = add_option(head, "Threads", OPT_SPIN, thread_value, NULL);
    head = add_option(head, "Hash", OPT_SPIN, hash_value, NULL);
    head = add_option(head, "Clear Hash", OPT_BUTTON, clear_hash_value, on_clear_hashtable);
    head = add_option(head, "Ponder", OPT_CHECK, ponder_value, NULL);
    head = add_option(head, "MultiPV", OPT_SPIN, multi_pv_value, NULL);
    head = add_option(head, "Skill Level", OPT_SPIN, skill_value, NULL);
    head = add_option(head, "Move Overhead", OPT_SPIN, move_overhead_value, NULL);
    head = add_option(head, "nodestime", OPT_SPIN, nodes_time_value, NULL);
    head = add_option(head, "UCI_Chess960", OPT_CHECK, uci_chess960_value, NULL);
    head = add_option(head, "UCI_AnalyseMode", OPT_CHECK, uci_analyze_mode_value, NULL);
    head = add_option(head, "UCI_LimitStrength", OPT_SPIN, uci_limit_strength_value, NULL);
    head = add_option(head, "UCI_Elo", OPT_SPIN, uci_elo_value, NULL);
    head = add_option(head, "UCI_ShowWDL", OPT_CHECK, uci_show_wdl_value, NULL);

    return (head);
}

////////////////////////////////////////////////////////////////
// MAIN MESSAGE HANDLER

/* handles the position command */
void handle_position(char *token, board_t *board) {
    const char delim[] = " \n\t";
    char fen[MAX_LEN];

    token = strtok(NULL, delim);

    if (!strcmp(token, "startpos")) {
        /* handles the starting position */
        load_by_FEN(board, STARTING_FEN);
        token = strtok(NULL, delim);
        if (token && strcmp(token, "moves")) {
            fprintf(stderr, "%sIncorrect syntax in the 'position startpos' command: %s%s\n",Color_PURPLE, token, Color_END);
            return;
        }
    } else if (!strcmp(token, "fen")) {
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
        fprintf(stderr, "%sIncorrect syntax in the 'position' command: %s%s\n",Color_PURPLE, token, Color_END);
    }

    /* finally, read all the moves, if there are any */
    if (token) 
        while((token = strtok(NULL, delim))) {
            move_t *move = str_to_move(board, token);
            if (move){
                play_move(board, move, board->player);
                board->ply_no++;
                free_move(move);
            }
            else {
                fprintf(stderr, "%sInvalid move: %s%s\n",Color_PURPLE, token, Color_END);
                exit(-1);
            }
        }

    return;
}

search_data* init_search_data(board_t *board){
    search_data *data = (search_data*) malloc(sizeof(search_data));
    data->board = copy_board(board);  /* current board */
    data->max_depth = -1;             /* maximum search depth in plies */
    data->max_seldepth = -1;          /* maximum search depth with quiescence search */
    data->max_nodes = -1;             /* maximum nodes allowed to search */
    data->max_time = -1;              /* maximum time allowed in ms */
    data->wtime = -1;                 /* white time increment in ms */
    data->btime = -1;                 /* black time increment in ms */
    data->winc = 0;                   /* white time increment in ms */      
    data->binc = 0;                   /* black time increment in ms */ 
    data->ponder = 0;                 /* tells engine to start search at ponder move */
    data->run_infinite = 0;           /* tells the engine to run aslong as stop != 1 */
    data->stop = 0;                   /* tells the engine to stop search when stop == 1*/
    data->best_move = NULL;           /* computed best move (so far) */

    return data;
}

void free_search_data(search_data *data){
    free(data->board);          
    free(data->best_move);
    free(data);
}

/* structure to exchange information to the background thread for search */

pthread_t game_thread = 0;

/* plays a move on the board passed and returns the best move */
void *start_search(void *args) {
    /* initialize options */
    search_data *data = (search_data *) args;
    clock_t begin;
    if (book_possible(data->board) == 1) {
        fprintf(stderr, "%sBook move possible. Searching...%s\n", Color_PURPLE, Color_END);
        begin = clock();
        data->best_move = get_random_book(data->board);
    } else {
        fprintf(stderr, "%sNo book move possible. Searching...%s\n", Color_PURPLE, Color_END);
        begin = clock();
        double max_time = (double) data->max_time/1000.0;
        data->best_move = iterative_search(data->board, data->max_depth, max_time);
    }

    /* output the best move to stdout */
    printf("bestmove ");
    print_LAN_move(data->best_move);
    printf("\n");

    fprintf(stderr, "%sTime: %f%s\n", Color_PURPLE,(double)(clock()-begin)/CLOCKS_PER_SEC, Color_END);
    /* signal communication thread that search is finished*/
    game_thread = 0;

    /* free search data including copy of board and the bestmove */
    free_search_data(data);

    /* exit search thread */
    pthread_exit(NULL);
}

/* handles the go command */
search_data* handle_go(char *token, board_t *board) {
    const char delim[] = " \n\t";
    search_data *data = init_search_data(board);

    while ((token = strtok(NULL, delim))) {
        if(!strcmp(token, "searchmoves")) {
            // TODO
            fprintf(stderr, "%sCant handle %s command yet!%s\n", Color_PURPLE, token, Color_END);
            exit(-1);
        }
        if(!strcmp(token, "ponder")) {
            data->ponder = 1;
        } else if(!strcmp(token, "wtime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for white%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->wtime = atoi(token);
            }
        } else if(!strcmp(token, "btime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for black%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->btime = atoi(token);
            }
        } else if(!strcmp(token, "winc")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for white increment%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->winc = atoi(token);
            }
        } else if(!strcmp(token, "binc")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing time left for black increment%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->binc = atoi(token);
            }
        } else if(!strcmp(token, "movestogo")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing moves to go%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                // TODO
            }
        } else if(!strcmp(token, "depth")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing depth%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->max_depth = atoi(token);
            }            
        } else if(!strcmp(token, "nodes")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing nodes%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->max_nodes = atoi(token);
            }
        } else if(!strcmp(token, "mate")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing mate number%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                // TODO
            }
        } else if(!strcmp(token, "movetime")) {
            token = strtok(NULL, delim);
            if (!token) {
                fprintf(stderr, "%sMissing movetime%s\n", Color_PURPLE, Color_END);
                exit(-1);
            } else {
                data->max_time = atoi(token);
            }
        } else if (!strcmp(token, "infinite")) {
            data->run_infinite = 1;
            break;
        } else {
            fprintf(stderr, "%sincorrect command: %s%s\n", Color_PURPLE, token, Color_END);
        }
    }

    while (game_thread) {
        fprintf (stderr, "%sSearch underway ... sleeping for 1 second%s\n", Color_PURPLE, Color_END);
        sleep (1);
    }

    pthread_create(&game_thread, NULL, start_search, (void *) data);

    return data;
}

/* runs the main loop of the the UCI communication interface */
void *main_interface_loop(void *args) {
    /* initialize options */
    board_t *board = (board_t *)args;
    options_t *options = init_options();
    search_data *data = NULL;

    /* removing buffering from stdin and stdout */
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    printf("id name AChess 0.1\nid author Alexander Herbrich\n\n");
    printf("uciok\n");

    while (1) {
        char buffer[MAX_LEN];
        char *token;
        const char delim[] = " \n\t";

        /* read a whole line from input */
        fgets(buffer, MAX_LEN - 1, stdin);

        /* get the first token */
        token = strtok(buffer, delim);

        if (!token) continue;

        if (!strcmp(token, "quit")) {
            /* handle 'quit' or 'stop' command */
            break;
        } else if(!strcmp(token, "stop")){
            if(data){
                data->stop = 1;
            }
        } else if (!strcmp(token, "uci")) {
            /* handle 'uci' command */
            printf("id name AChess 0.1\nid author Alexander Herbrich\n\n");
            printf("uciok\n");
        } else if (!strcmp(token, "setoption")) {
            /* handle 'setoption' command */
            printf("setoption\n");
        } else if (!strcmp(token, "go")) {
            /* handle 'go' command */
            data = handle_go(token, board);
        } else if (!strcmp(token, "position")) {
            /* handle 'position' command */
            handle_position(token, board);
        } else if (!strcmp(token, "ucinewgame")) {
            /* handle the 'ucinewgame' command */
            clear_board(board);
            clear_hashtable();
        } else if (!strcmp(token, "isready")) {
            /* handle 'isready' command */
            printf("readyok\n");
        }
    }
    free_options(options);

    return (NULL);
}

////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT
int main() {
    pthread_t comm_thread;

    /* Initialize all dictionaries and tables */
    board_t *board = init_board();
    init_zobrist();
    init_hashtable();
    init_book();

    pthread_create(&comm_thread, NULL, main_interface_loop, (void *)board);
    pthread_join(comm_thread, NULL);

    /* free the board */
    free_board(board);
}
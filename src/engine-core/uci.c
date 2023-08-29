#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

#include "include/engine-core/uci.h"

#include "include/engine-core/types.h"
#include "include/engine-core/search.h"
#include "include/engine-core/board.h"
#include "include/engine-core/move.h"
#include "include/engine-core/pq.h"
#include "include/engine-core/prettyprint.h"


#define BUFFER_SIZE 16384
#define TO_PROM_FLAG(X) ((X== 'n' || X == 'N') ? KPROM : (X == 'b' || X == 'B') ? BPROM : (X == 'r' || X == 'R') ? RPROM : (X == 'q' || X == 'Q') ? QPROM : -1)
#define VALID_PROM_FLAG(X) (X != -1)

int verbosity = 0;
int search_running = 0;

/* ------------------------------------------------------------------------------------------------ */
/* functions for option and engine info handling                                                    */
/* ------------------------------------------------------------------------------------------------ */

/* initializes the options */
options_t init_options(void){
    options_t options = {
        .opt_hash = {.min = 1, .max = 1024, .def = 64}
    };

    return options;
}

/* initializes the engine info */
engine_info_t init_engine_info(void){
    engine_info_t engine_info = {
        .name = "Herby",
        .author = "Alexander Herbrich",
        .version = "v2.0",
    };

    return engine_info;
}


/* ------------------------------------------------------------------------------------------------ */
/* helper functions for uci                                                                         */
/* ------------------------------------------------------------------------------------------------ */

/* prints a string to stderr if verbosity is set to high */
void verbosity_print(char* str){
    if(verbosity > 0) fprintf(stderr, "[%sINFO%s] %s\n", Color_PURPLE, Color_END, str);
}

/* converts a string to lower case */
char* to_lower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}


/* ------------------------------------------------------------------------------------------------ */
/* functions for managing uci interface                                                             */
/* ------------------------------------------------------------------------------------------------ */

/* Converts a move string in LANotation to a move */
move_t *LAN_to_move(board_t *board, char *move_str) {
    /* exit if we would read into uninitialized memory */
    if (strlen(move_str) < 4) return NULL;

    /* exit if move string is too long */
    if(strlen(move_str) > 5) return NULL;

    /* extract the from and to field and promotion piece of the move */
    int file_from = move_str[0] - 'a';
    int rank_from = move_str[1] - '1';
    int file_to = move_str[2] - 'a';
    int rank_to = move_str[3] - '1';
    int prom_flag = (strlen(move_str) == 5) ? TO_PROM_FLAG((move_str[4])) : 0b0000;

    /* exit if files and ranks are invalid or if prom square is invalid */
    if( file_from < 0 || file_from > 7 || rank_from < 0 || rank_from > 7 ||
        file_to < 0 || file_to > 7 || rank_to < 0 || rank_to > 7 ||
        (prom_flag != 0b0000 && !VALID_PROM_FLAG(prom_flag))){
        return NULL;
    }

    /* converts the from and to position to an index */
    idx_t from = 8 * rank_from + file_from;
    idx_t to = 8 * rank_to + file_to;

    /* generate all possible moves in the current position */
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    /* check if the move described by the move string is a valid move, i.e. in the move list */
    for (int i = 1; i < (&movelst)->nr_elem + 1; i++) {
        move_t* move = &movelst.array[i];
        if (move->from == from && move->to == to){
            /* if move is matches a non-promotion, we are finished */
            if(prom_flag == 0b0000 && (move->flags & 0b1000) == 0b0000) {
                move = copy_move(move);
                return move;
            }
            /* if move matches a promotion */ 
            else if (prom_flag != 0b0000 && (move->flags & 0b1000) != 0){
                move = copy_move(move);
                /* we | to ensure we copy capture bit, which might be set */
                move->flags = (move->flags & 0b0100) | prom_flag;    
                return move;
            }
        }

    }

    return NULL;
}

/* Starts the search intitiated by user/gui */
void *start_search(void *args) {
    searchdata_t *searchdata = (searchdata_t *)args;

    /* indicate that search is running */
    search_running = 1;

    /* start iterative search */
    search(searchdata);

    /* inidiacte that search is finished */
    search_running = 0;

    pthread_exit(NULL);
}

/* prints the UCI command response */
void uci_command_response(uci_args_t* uci_args) {
    /* print engine info */
    engine_info_t engine_info = uci_args->engine_info;
    printf("id name %s %s\n", engine_info.name, engine_info.version);
    printf("id author %s\n", engine_info.author);
    printf("\n");

    /* print options */
    printf("option name Hash type spin default %d min %d max %d", uci_args->options.opt_hash.def, uci_args->options.opt_hash.min, uci_args->options.opt_hash.max);

    /* print uciok to indicate that engine is ready */
    printf("\n");
    printf("uciok\n");
}

/* handles and prints the verbosity command response */
void verbosity_command_response(void){
    char* level = strtok(NULL, " \n\t");
    if(!level) {
        verbosity_print("no level given - must be one of: low, medium, high");
        return;
    }
    if(!strcmp(level, "low")){
        verbosity = 0;
    } else if(!strcmp(level, "high")){
        verbosity = 1;
        verbosity_print("verbosity set to high");
    } else {
        verbosity_print("verbosity level must be one of: low, high");
    }
}

/* handles and prints the setoption command response */
void setoption_command_response(options_t options){
    char* option = strtok(NULL, " \n\t");
    /* if no option given exit */
    if(!option) { 
        verbosity_print("no option given"); 
        return; 
    }
    /* handle case insensitivity */
    option = to_lower(option);

    /* handle options */
    if(!strcmp(option, "hash")){
        char* value_str = strtok(NULL, " \n\t");
        if(!value_str) { 
            verbosity_print("no value given"); 
            return; 
        }

        int value = atoi(value_str);

        /* check if value lies in acceptable range */
        if(value < options.opt_hash.min || value > options.opt_hash.max){
            verbosity_print("value out of range - use 'uci' for more information "); 
            return;
        }

        /* TODO - restrict hash table size */
        verbosity_print("(TODO: hashtable size set accordingly). Currently nothing changes.");

    } else{
        verbosity_print("there exist no such option!");
    }
}

/* handles and prints the ucinewgame command response */
void ucinewgame_command_response(board_t* board){
    clear_board(board);
    verbosity_print("new game started");
}

/* handles and prints the position command response */
void position_command_response(board_t* board){
    char* fen_indicator = strtok(NULL, " \n\t");
    /* exit if no fen/startpos given (i.e only spaces, newlines  and tabs) */
    if(!fen_indicator) { 
        verbosity_print("no fen/startpos given"); 
        return; 
    }

    /* HANDLE POSITION STRING */
    /* if 'position startpos' command given */
    if(!strcmp(fen_indicator, "startpos")){
        /* set board into start position */
        load_by_FEN(board,
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        verbosity_print("board set to startpos");
    }
    /* if 'position fen' command given */ 
    else if (!strcmp(fen_indicator, "fen")){
        char fen[256];
        int fen_idx = 0;
        char* fen_str;

        /* extract fen string */
        for(int i = 0; i < 6; i++){
            fen_str = strtok(NULL, " \n\t");
            if(!fen_str) { 
                verbosity_print("fen specified incorrectly - follow sceme <pos> <color> <castle> <enpassant> <halfmove> <fullmove>"); 
                return; 
            }
            int len = strlen(fen_str);
            strcpy(&(fen[fen_idx]), fen_str);
            fen_idx += len;
            fen[fen_idx++] = ' ';
        }
        fen[fen_idx] = '\0';
        
        /* set board by fen */
        load_by_FEN(board, fen);
        verbosity_print("board set by fen");
    }
    /* exit if fen/startpos wronlgy specified */ 
    else {
        verbosity_print("unknown fen indicator - use 'startpos' or 'fen'");
        return;
    }

    /* finally, check if we have to parse further moves */
    char* moves_indicator = strtok(NULL, " \n\t");
    if(moves_indicator){
        /* if there are moves given, read all the moves */
        if(!strcmp(moves_indicator, "moves")){
            /* parse move after move and play it if possible, otherwise abort parsing and reset board */
            char* move_str = strtok(NULL, " \n\t");

            if(!move_str) { 
                clear_board(board);
                verbosity_print("no moves given - board has been reset to empty"); 
                return; 
            }

            while (move_str) {
                move_t *move = LAN_to_move(board, move_str);
                if (move) {
                    do_move(board, *move);
                    free_move(move);
                } else {
                    clear_board(board);
                    verbosity_print("invalid move - board has been reset to empty");
                    return;
                }
                move_str = strtok(NULL, " \n\t");
            }

            verbosity_print("all moves played successfully!");
        } 
        /* exit if moves are not signaled by 'moves' keyword */
        else{
            clear_board(board);
            verbosity_print("unknown move indicator - use keyword 'moves' - board has been reset to empty");
            return;
        }
    }
    
}

/* handles and prints the go command response */
int go_command_response(searchdata_t* searchdata, pthread_t* search_thread){
    char* token = strtok(NULL, " \n\t");

    /* if no specification given, search infinite */
    if(!token) { 
        verbosity_print("no specification given - searching infinite");
        pthread_create(search_thread, NULL, start_search, (void *)searchdata);
        return 0; 
    }

    /* if help command given, print help */
    if(!strcmp(token, "help")){
        verbosity_print("go command syntax: go [wtime <ms>] [btime <ms>] [winc <ms>] [binc <ms>] [movetime <ms>] [depth <depth>] [infinite]");
        return 1;
    }

    /* else parse go command arguments/specifications */
    while(token){
        if (!strcmp(token, "searchmoves")){
            /* TODO - implement searchmoves */
            verbosity_print("searchmoves not yet implemented");
        } else if (!strcmp(token, "ponder")){
            /* TODO - implement ponder */
            verbosity_print("ponder not yet implemented");
        } else if(!strcmp(token, "wtime")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no wtime given"); 
                return 1; 
            } else {
                searchdata-> timer.wtime = atoi(token);
                searchdata-> timer.run_infinite = 0;
            }
        } else if (!strcmp(token, "btime")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no btime given"); 
                return 1; 
            } else {
                searchdata-> timer.btime = atoi(token);
                searchdata-> timer.run_infinite = 0;
            }
        } else if (!strcmp(token, "winc")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no winc given"); 
                return 1; 
            } else {
                searchdata-> timer.winc = atoi(token);
                searchdata-> timer.run_infinite = 0;
            }
        } else if (!strcmp(token, "binc")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no binc given"); 
                return 1; 
            } else {
                searchdata-> timer.binc = atoi(token);
                searchdata-> timer.run_infinite = 0;
            }
        } else if (!strcmp(token, "movestogo")){
            /* TODO - implement movestogo */
            verbosity_print("movestogo not yet implemented");
            /* skip value of movetogo */
            token = strtok(NULL, " \n\t");
            fprintf(stderr, "%s\n", token);
            if(!token) return 1; 
        } else if (!strcmp(token, "depth")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no depth given"); 
                return 1; 
            } else {
                searchdata-> timer.max_depth = atoi(token);
            }
        } else if (!strcmp(token, "nodes")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no nodes given"); 
                return 1; 
            } else {
                searchdata-> timer.max_nodes = atoi(token);
            }
        } else if (!strcmp(token, "mate")){
            /* TODO - implement mate */
            verbosity_print("mate not yet implemented");
        } else if (!strcmp(token, "movetime")){
            token = strtok(NULL, " \n\t");
            if(!token) { 
                verbosity_print("no movetime given"); 
                return 1; 
            } else {
                searchdata-> timer.max_time = atoi(token);
                searchdata-> timer.run_infinite = 0;
            }
        } else if (!strcmp(token, "infinite")){
            /* get next token and continue */
            token = strtok(NULL, " \n\t");
            continue;
        } else {
            verbosity_print("unknown command - use 'go help' for more information");
            return 1;
        }
        token = strtok(NULL, " \n\t");
    }

    verbosity_print("searching ...");
    pthread_create(search_thread, NULL, start_search, (void *)searchdata);
    return 0; 
}

/* runs the main loop of the the UCI communication interface */
void uci_interface_loop(void *args) {
    /* extract arguments */
    uci_args_t* uci_args = (uci_args_t *) args;
    board_t* board = uci_args->board;
    searchdata_t* searchdata = uci_args->searchdata;
    engine_info_t engine_info = uci_args->engine_info;
    options_t options = uci_args->options;

    /* set verbosity, default low */
    verbosity = 1;
    /* set search running to false */
    search_running = 0;

    /* remove buffering from stdin and stdout */
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    /* print (reduced) chess engine info at startup */
    printf("%s %s by %s\n", engine_info.name, engine_info.version, engine_info.author);

    /* initialize search thread */
    pthread_t search_thread = NULL;

    /* start the main loop */
    while(1){
        char buffer[BUFFER_SIZE];

        /* read a whole line from input */
        if(fgets(buffer, BUFFER_SIZE - 1, stdin) == NULL){
            fprintf(stderr, "Error reading from stdin\n");
            break;
        };
        
        /* check for buffer overflow */
        char* ptr = strchr(buffer, '\n');
        if(ptr == NULL){
            /* if buffer overflow detected */
            /* (1) clear stdin */ 
            char c;
            do c = getchar(); while (c != '\n' && c != EOF);

            /* (2) and exit */
            fprintf(stderr, "Error: Input line too long\n");
            break;
        } 


        /* get the first command */
        char* command = strtok(buffer, " \n\t");

        /* if no command read (i.e. only space, newline or tab), continue reading */
        if (!command) continue;
        
        /* else, parse the command */
        if (!strcmp(command, "uci")) {
            uci_command_response(uci_args);
        } else if (!strcmp(command, "isready")) {
            printf("readyok\n");
        } else if (!strcmp(command, "verbosity")){
            verbosity_command_response();
        } else if (!strcmp(command, "setoption")){
            setoption_command_response(options);
        } else if (!strcmp(command, "ucinewgame")){
            ucinewgame_command_response(board);
        } else if(!strcmp(command, "position") && !search_running){
            position_command_response(board);
        } else if(!strcmp(command, "go") && !search_running){
            if(searchdata) free_search_data(searchdata);
            searchdata = init_search_data(board);
            go_command_response(searchdata, &search_thread);
        } else if (!strcmp(command, "stop")) {
            if(searchdata) searchdata->timer.stop = 1;
        } 
        else if(!strcmp(command, "quit")){
            break;
        } else {
            verbosity_print("unknown command or search already running - if latter use 'stop' to stop it");
        } 
    }

    return;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/engine-core/engine.h"
#include "include/ordering/factors.h"
#include "include/ordering/gaussian.h"
#include "include/ordering/ordering.h"
#include "include/ordering/urgencies.h"
#include "include/parse/parse.h"

/* type for storing a set of moves and all other possible moves */
typedef struct _move_set_t {
    int move_idx;       /* index of the move in the game */
    int no_moves;       /* total number of all moves (including the move made) */
    int* move_keys;     /* set of all moves (where move at index 0 is the move made) */
} move_set_t;

/* allocates a new move set */
move_set_t* new_move_set(int no_moves, int move_idx) {
    move_set_t* ms = (move_set_t*) malloc(sizeof(move_set_t));
    ms->move_keys = (int*) malloc(sizeof(int) * no_moves);
    ms->no_moves = no_moves;
    ms->move_idx = move_idx;
    return ms;
}

/* frees the memory for a move set */
void delete_move_set(move_set_t *ms) {
    if(ms) {
        free(ms->move_keys);
        free(ms);
    }
    return;
}

/* type for storing sets of move sets */
typedef struct _move_set_array_t {
    int no_move_sets;          /* total number of move sets */
    int capacity;              /* total capacity of the array of move sets */
    move_set_t** move_sets;    /* array of all move sets */
} move_set_array_t;

/* allocates a new array of move sets */
move_set_array_t* new_move_set_array(void) {
    move_set_array_t* msa = (move_set_array_t*) malloc(sizeof(move_set_array_t));
    msa->move_sets = NULL;
    msa->no_move_sets = 0;
    msa->capacity = 0;
    return msa;
}

/* adds a new move set to the array of move sets */
void add_move_set(move_set_array_t* msa, move_set_t* ms) {
    if(msa) {
        if(msa->no_move_sets == msa->capacity) {
            msa->capacity = (msa->capacity == 0) ? 1 : 2 * msa->capacity;
            msa->move_sets = (move_set_t**) realloc(msa->move_sets, sizeof(move_set_t*) * msa->capacity);
        }
        msa->move_sets[msa->no_move_sets++] = ms;
    }
    return;
}

/* frees a given array of move sets */
void delete_move_set_array(move_set_array_t* msa) {
    if(msa) {
        for(int i = 0; i < msa->no_move_sets; i++) {
            delete_move_set(msa->move_sets[i]);
        }
        free(msa->move_sets);
        free(msa);
    }
    return;
}

/* converts a set of games to a set of move sets */
move_set_array_t* games_to_move_sets(chess_games_t chess_games) {
    move_set_array_t* msa = new_move_set_array();

    for(int i = 0; i < chess_games.no_games; i++) {
        /* create an empty board to re-play the game */
        board_t* board = init_board();
        load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        /* create a local copy of the move string to parse it */
        char* move_str = (char*) malloc(strlen(chess_games.games[i]->move_list) + 1);
        strcpy(move_str, chess_games.games[i]->move_list);

        int move_idx = 1;
        char* token = strtok(move_str, " ");
        do {
            move_t* move = str_to_move(board, token);
            if (move) {
                /* generate all possible moves */
                maxpq_t move_lst;
                initialize_maxpq(&move_lst);
                generate_moves(board, &move_lst);

                /* create array to hold all move keys corresponding to possible moves */
                int no_moves = move_lst.nr_elem;
                move_set_t *ms = new_move_set(no_moves, move_idx);
                int idx = 0;

                /* extract move key from move made */
                ms->move_keys[idx++] = calculate_move_key(board, move);

                /* extract move key from all other moves */
                move_t* other_move;
                while ((other_move = pop_max(&move_lst)) != NULL) {
                    /* if we see move made, skip it */
                    if (is_same_move(move, other_move)) {
                        free_move(other_move);
                        continue;
                    }

                    /* else extract ranking-info from move */
                    ms->move_keys[idx++] = calculate_move_key(board, other_move);

                    free_move(other_move);
                }
                /* add the move set to the set of move set and increment move index */
                add_move_set(msa, ms);
                move_idx++;

                /* execute move made */
                do_move(board, move);
                free_move(move);
            } else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n", Color_PURPLE, token, Color_END);
                exit(-1);
            }
        } while ((token = strtok(NULL, " ")));

        /* free the temporary copy of the move string (after parsing with strtrok) */
        free(move_str);
        /* free the board after every move has been played */
        free_board(board);
    }

    return msa;
}

/* trains a Bayesian ranking model from the replay of the games */
void train_model(chess_game_t** chess_games, int nr_of_games, train_info_t train_info) {
    /* parameters needed for full inference/training */
    ranking_update_info_t* ranking_updates = NULL;
    int no_gaussian = 0, no_factors = 0;

    /* print some information on the screen */
    if (train_info.verbosity >= 1) {
        fprintf(stderr, "Training started (%s)\n", (train_info.full_training) ? "full" : "incremental");
    }

    chess_games_t games = { .games=chess_games, .no_games=nr_of_games };
    move_set_array_t* msa = games_to_move_sets(games);

    /* play games */
    for (int i = 0; i < msa->no_move_sets; i++) {
        int no_moves = msa->move_sets[i]->no_moves;

        // retrieve the urgency beliefs for all moves
        gaussian_t* urgencies_ptr[no_moves];
        for (int j = 0; j < no_moves; j++) {
            urgencies_ptr[j] = get_urgency(train_info.ht_urgencies, msa->move_sets[i]->move_keys[j]);
            if (urgencies_ptr[j] == NULL) {
                urgencies_ptr[j] = add_urgency(train_info.ht_urgencies, msa->move_sets[i]->move_keys[j], train_info.prior);
            }
        }

        // execute_ranking update
        if (train_info.full_training) {
            ranking_updates = add_ranking_update_graph(ranking_updates, urgencies_ptr, no_moves, train_info.beta * train_info.beta);
            no_factors += (3 * no_moves - 2);
            no_gaussian += (8 * no_moves - 5);
        } else {
            update(urgencies_ptr, no_moves, train_info.beta * train_info.beta);
        }
    }

    /* now perform the full training and release the small update (factor) graphs */
    if (train_info.full_training) {
        if (train_info.verbosity >= 1) {
            fprintf(stderr, "Full training started\n\tNo. of factors: %d\n\tNo. of Gaussian: %d\n", no_factors, no_gaussian);
        }
        refresh_update_graph(ranking_updates, 5e-1, train_info.base_filename);
        delete_ranking_update_graphs(ranking_updates);
    }

    /* free the move set array */
    delete_move_set_array(msa);

    return;
}

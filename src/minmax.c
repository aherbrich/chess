#include "../include/chess.h"
#include "../include/eval.h"
#include "../include/zobrist.h"
#include "../include/prettyprint.h"

/* global counters for evaluation */
int nodes_searched = 0;
int hash_used = 0;
int hash_bounds_adjusted = 0;


int quietSearch(board_t *board, int alpha, int beta, clock_t start, double time_left) {
    // evaluate board
    int value = eval_board(board);

    // beta cutoff
    if (value >= beta) {
        return value;
    }

    node_t *move_list = generate_captures(board);
    move_list = sort_moves(move_list);

    // if no more captures possible return evaluation
    if (len(move_list) == 0) {
        // else return evaluation
        return value;
    }

    move_t *move;
    player_t player_at_turn = board->player;
    int best_value = value;

    // alpha beta only on captures
    while ((move = pop(move_list)) != NULL) {
        nodes_searched++;

        double time_diff = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (time_diff > time_left) {
            free_move(move);
            free_move_list(move_list);
            return get_eval_from_hashtable(board);
        }

        alpha = max(value, alpha);

        play_move(board, move, player_at_turn);
        int value = -quietSearch(board, -beta, -alpha, start, time_left);
        reverse_move(board, move, player_at_turn);

        if (value > best_value) {
            best_value = value;
            if (best_value >= beta) {
                free_move(move);
                goto beta_cutoff;
            }
        }
        free_move(move);
    }
beta_cutoff:

    // free the remaining move list
    while ((move = pop(move_list)) != NULL) {
        free_move(move);
    }
    free(move_list);

    return best_value;
}

int alphaBeta_with_TT(board_t *board, uint8_t depth, int alpha, int beta, clock_t start, double time_left) {
    int old_alpha = alpha;
    move_t *best_move_so_far = NULL;

    move_t *pv_move = NULL;
    int16_t pv_value;
    int8_t pv_flags;
    int8_t pv_depth;

    int entry_found = get_hashtable_entry(board, &pv_flags, &pv_value, &pv_move, &pv_depth);

    if (entry_found && pv_depth == depth) {
        nodes_searched++;
        if (pv_flags == FLG_EXCACT) {
            hash_used++;
            free_move(pv_move);
            return pv_value;
        } else if (pv_flags == FLG_CUT) {
            hash_bounds_adjusted++;
            alpha = max(pv_value, alpha);
        } else if (pv_flags == FLG_ALL) {
            hash_bounds_adjusted++;
            beta = min(pv_value, beta);
        }
        if (alpha >= beta) {
            hash_used++;
            free_move(pv_move);
            return pv_value;
        }
    }

    if (depth == 0) {
        nodes_searched++;
        free_move(pv_move);
        return (quietSearch(board, alpha, beta, start, time_left));
        // return eval_board(board);
    }

    move_t *move;
    player_t player_at_turn = board->player;
    int best_value;

    node_t *move_list = generate_moves(board);
    move_list = sort_moves(move_list);

    if (entry_found && PVMove_is_possible(move_list, pv_move)) {
        nodes_searched++;

        double time_diff = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (time_diff > time_left) {
            free_move(move);
            free_move_list(move_list);
            return (entry_found) ? pv_value : -16000;
        }

        play_move(board, pv_move, player_at_turn);
        best_value = -alphaBeta_with_TT(board, depth - 1, -beta, -alpha, start, time_left);
        reverse_move(board, pv_move, player_at_turn);

        free_move(best_move_so_far);
        best_move_so_far = copy_move(pv_move);
        if (best_value >= beta) {
            goto beta_cutoff;
        }
    } else {
        best_value = NEGINFINITY;
    }

    if (len(move_list) == 0) {
        nodes_searched++;
        free_move(pv_move);
        free_move(best_move_so_far);
        free_move_list(move_list);
        return eval_end_of_game(board, depth);
    }

    while ((move = pop(move_list)) != NULL) {
        nodes_searched++;

        double time_diff = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (time_diff > time_left) {
            free_move(move);
            free_move_list(move_list);
            return (entry_found) ? pv_value : -16000;
        }

        if (pv_move != NULL && is_same_move(move, pv_move)) {
            free_move(move);
            continue;
        }

        alpha = max(best_value, alpha);
        play_move(board, move, player_at_turn);
        int value = -alphaBeta_with_TT(board, depth - 1, -beta, -alpha, start, time_left);
        reverse_move(board, move, player_at_turn);

        if (value > best_value) {
            best_value = value;
            free_move(best_move_so_far);
            best_move_so_far = copy_move(move);

            if (best_value >= beta) {
                free_move(move);
                goto beta_cutoff;
            }
        }
        free_move(move);
    }

    beta_cutoff:

    if (best_value <= old_alpha) {
        store_hashtable_entry(board, FLG_ALL, best_value, best_move_so_far, depth);
    } else if (best_value >= beta) {
        store_hashtable_entry(board, FLG_CUT, best_value, best_move_so_far, depth);
    } else {
        store_hashtable_entry(board, FLG_EXCACT, best_value, best_move_so_far, depth);
    }

    /* free the remaining move list */
    free_move_list(move_list);

    /* free tt_move & best_move_so_far */
    free_move(pv_move);
    free_move(best_move_so_far);

    return best_value;
}

move_t *iterative_search(board_t *board, int8_t max_depth, double maxtime) {
    /* reset the performance counters */
    nodes_searched = 0;
    hash_used = 0;
    hash_bounds_adjusted = 0;

    int maxdepth = (max_depth == -1)?100:max_depth;
    double time_left = maxtime;
    move_t *best_move = NULL;
    clock_t begin = clock();
    
    clear_hashtable();

    for (int i = 1; i <= maxdepth; i++) {
        clock_t begin_eval = clock();
        int evaluation = alphaBeta_with_TT(board, i, NEGINFINITY, INFINITY, begin_eval, time_left);

        free_move(best_move);
        best_move = get_best_move_from_hashtable(board);
        clock_t end = clock();

        time_left -= (double)(end - begin_eval) / CLOCKS_PER_SEC;

        printf("info score cp %d depth %d nodes %d time %d pv ", evaluation, i, nodes_searched, (int) ((double)(end - begin) / CLOCKS_PER_SEC*1000));
        print_line(board, i);
        printf("\n");

        
        if (time_left <= 0) {
            fprintf(stderr, "%sDepth searched: %d%s\n", Color_PURPLE,i,Color_END);
            goto search_finished;
        }
    }

    search_finished:

    return best_move;
}
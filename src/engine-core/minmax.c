#include <stdio.h>
#include <sys/time.h>

#include "include/engine-core/eval.h"
#include "include/engine-core/move.h"
#include "include/engine-core/prettyprint.h"
#include "include/engine-core/search.h"
#include "include/engine-core/types.h"
#include "include/engine-core/zobrist.h"
#include "include/ordering/ordering.h"
#include "include/ordering/urgencies.h"

#define TOLERANCE 15  // ms
#define STOP_ACCURACY 1
#define MAXDEPTH 100
#define WINDOWSIZE 50

int pv_move_searches = 0;
int pv_move_hits_prob = 0;
int pv_move_hits_mean = 0;

int last_check = 0;
int stop_immediately = 0;

int delta_in_ms(searchdata_t *searchdata) {
    gettimeofday(&(searchdata->end), 0);
    return (int)((searchdata->end.tv_sec - searchdata->start.tv_sec) * 1000.0f +
                 (searchdata->end.tv_usec - searchdata->start.tv_usec) /
                     1000.0f);
}

/* Creates score string for info output (for GUI) */
char *get_mate_or_cp_value(int score, int depth) {
    char *buffer = (char *)malloc(1024);
    for (int i = 0; i < 1024; i++) buffer[i] = '\0';

    if (score >= 16000) {
        sprintf(buffer, "mate %d", (depth / 2));
    } else if (score <= -16000) {
        sprintf(buffer, "mate %d", -(depth / 2));
    } else {
        sprintf(buffer, "cp %d", score);
    }
    return buffer;
}

/* Determines a draw by threefold repitiion */
int draw_by_repition(board_t *board) {
    uint64_t current_board_hash = board->hash;

    int counter = 0;
    for (int i = 0; i < board->ply_no; i++) {
        if (board->history[i].hash == current_board_hash) counter++;
        if (counter == 2) {
            return 1;
        }
    }
    return 0;
}

/* Determines how much time is available for search (search parameters specified
 * by the caller (the gui)) */
int calculate_time(searchdata_t *data) {
    int time_available = 0;
    int atleast_one_time_found = 0;
    int time_available_movetime = 0;
    int time_available_remainingtime = 0;

    // if max time for move is given
    if (data->max_time != -1) {
        // calculate time avaiable for search
        time_available_movetime += data->max_time;
        time_available_movetime = (data->board->player == WHITE)
                                      ? (time_available_movetime + data->winc)
                                      : (time_available_movetime + data->binc);
        time_available_movetime -= TOLERANCE;

        time_available = time_available_movetime;
        // make sure chess engine has atleast 5ms for search
        if (time_available < 5) {
            time_available = 5;
        }
        atleast_one_time_found = 1;
    }
    // if whites/blacks remaining time is given
    if ((data->board->player == WHITE && data->wtime != -1) ||
        (data->board->player == BLACK && data->btime != -1)) {
        // calculate time avaiable for search
        time_available_remainingtime =
            (data->board->player == WHITE)
                ? ((int)((double)data->wtime / 30.0))
                : ((int)((double)data->btime / 30.0));
        time_available_remainingtime =
            (data->board->player == WHITE)
                ? (time_available_remainingtime + data->winc)
                : (time_available_remainingtime + data->binc);
        time_available_remainingtime -= TOLERANCE;

        // if no max time was given or the now calculated time is lower than the
        // max time given update it
        if (!atleast_one_time_found ||
            time_available_remainingtime < time_available_movetime) {
            time_available = time_available_remainingtime;
            // again, make sure chess engine has atleast 5ms for search
            if (time_available < 5) {
                time_available = 5;
            }
        }
        atleast_one_time_found = 1;
    }

    if (atleast_one_time_found) {
        // return the minimum of both times calculated (or one if only one was
        // given)
        return time_available;
    } else {
        // -1, indicates that no time limit was specified in the call
        return -1;
    }
}

/* Determines if the search has to be stopped */
/* Because of either (1) a STOP request or (2) we have used up our time to
 * search */
int search_has_to_be_stopped(searchdata_t *search_data) {
    // if a stop was initiaited, stop the search immediately
    if (search_data->stop) {
        return 1;
    }
    // or, if search is not in infinite mode and the time has run out, stop
    // search immediately
    if (!search_data->run_infinite) {
        if (delta_in_ms(search_data) >= search_data->time_available) {
            return 1;
        }
    }
    return 0;
}

/* Quescience search */
int quiet_search(board_t *board, int alpha, int beta,
                 searchdata_t *searchdata, int depth) {
    searchdata->nodes_searched++;
    searchdata->max_seldepth = (searchdata->max_seldepth < depth) ? depth : searchdata->max_seldepth;
    int eval = eval_board(board);

    if (eval >= beta) {
        return eval;
    }
    if (eval > alpha) {
        alpha = eval;
    }

    int best_eval = eval;

    // generate legal moves
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);
    move_t *move;

    while ((move = pop_max(&movelst))) {
        // every so often check if we have to stop the search
        if (last_check > STOP_ACCURACY && !stop_immediately) {
            last_check = 0;
            stop_immediately = search_has_to_be_stopped(searchdata);
        }

        // exit search cleanly if we have to stop
        if (stop_immediately) {
            free_move(move);
            free_pq(&movelst);
            return get_eval_from_hashtable(board);
        }

        // filter out non-captures
        if (!(move->flags & 0b0100)) {
            free_move(move);
            continue;
        }

        last_check++;

        // delta pruning
        // if (best_eval - 200 - is_capture(move->to, board) > eval) {
        //     free_move(move);
        //     continue;
        // }

        do_move(board, move);
        eval = -quiet_search(board, -beta, -alpha, searchdata, depth + 1);
        undo_move(board, move);

        // if eval is better than the best so far, update it
        if (eval > best_eval) {
            best_eval = eval;
        }
        // if eval is better than alpha, adjust bound
        if (eval > alpha) {
            alpha = eval;
        }
        // beta-cutoff
        if (eval >= beta) {
            free_move(move);
            break;
        }
        free_move(move);
    }

    free_pq(&movelst);

    return best_eval;
}

int negamax(searchdata_t *searchdata, int depth, int alpha, int beta) {
    searchdata->nodes_searched++;

    // Every so often, check if our time has expired
    if ((searchdata->nodes_searched & STOP_ACCURACY) == 0) {
        stop_immediately = search_has_to_be_stopped(searchdata);
    }

    // If we have to stop, exit search by returning 0 in all branches.
    // We will simply use the information of last search as our result
    // and discard any information gained in this search.
    if (stop_immediately) {
        return 0;
    }

    // Check for draw by repitition or fifty move rule
    if ((searchdata->board->history[searchdata->board->ply_no].fifty_move_counter >= 100 &&
         !(is_in_check(searchdata->board))) ||
        draw_by_repition(searchdata->board)) {
        return 0;
    }

    // If we've reached a depth of zero, evaluate the board
    if (depth == 0) {
        // return quiet_search(searchdata->board, alpha, beta, searchdata, 0);
        return eval_board(searchdata->board);
    }

    // ================================================================ //
    // TRANSPOSITION TABLE PROBING: Probe the transpotition table to    //
    // check if we've encountered this position before. If the probe is //
    // useable, meaning we've encountered this position at same depth   //
    // before and found an exact evaluation, return the score and stop  //
    // searching.                                                       //
    // ================================================================ //
    move_t *pv_move = NULL;
    int16_t pv_value;
    int8_t pv_flags, pv_depth;

    int entry_found = get_hashtable_entry(searchdata->board, &pv_flags,
                                          &pv_value, &pv_move, &pv_depth);

    if (entry_found && pv_depth == depth && pv_flags == EXACT) {
        free_move(pv_move);
        return pv_value;
    } else if (entry_found && pv_depth == depth && pv_flags == LOWERBOUND) {
        searchdata->hash_bounds_adjusted++;
        if (pv_value > alpha) alpha = pv_value;
    } else if (entry_found && pv_depth == depth && pv_flags == UPPERBOUND) {
        searchdata->hash_bounds_adjusted++;
        if (pv_value < beta) beta = pv_value;
    }
    if (alpha >= beta) {
        // early beta cutoff
        searchdata->hash_used++;
        free_move(pv_move);
        return pv_value;
    }

    // ================================================================== //
    // MOVE ITERATION: A value is associated with each position of the    //
    // game. This value is computed by means of an evaluation function    //
    // and it indicates how good it would be for a player to reach that   //
    // position. The player then iterates though all moves and chooses    //
    // the move that maximizes the minimum value of the position          //
    // resulting from the opponent's possible following moves.            //
    // ================================================================== //
    maxpq_t movelst;
    move_t *move;
    initialize_maxpq(&movelst);
    generate_moves(searchdata->board, &movelst);

    // extract move order urgency prediction
    int move_keys[movelst.nr_elem];
    double means[movelst.nr_elem];
    double probs[movelst.nr_elem];

    int idx_of_max_prob = 1;
    int idx_of_max_mean = 1;

    for (int i = 1; i <= movelst.nr_elem; i++) {
        move_keys[i - 1] = calculate_move_key(searchdata->board, movelst.array[i]);
        gaussian_t *g_ptr = get_urgency(ht_urgencies, move_keys[i - 1]);
        means[i - 1] = mean((g_ptr) ? *g_ptr : init_gaussian1D_standard_normal());
        probs[i - 1] = 0.0;
    }

    // predict_move_probabilities(ht_urgencies, probs, move_keys, movelst.nr_elem, 0.5 * 0.5);

    for (int i = 1; i <= movelst.nr_elem; i++) {
        if (probs[i - 1] >= probs[idx_of_max_prob - 1]) {
            idx_of_max_prob = i;
        }
        if (means[i - 1] >= means[idx_of_max_mean - 1]) {
            idx_of_max_mean = i;
        }
    }

    if (searchdata->nodes_searched % 300000 == 0) {
        printf("================================\n");
        // printf("| Hits/Searched(Prob):\n| %d/%d\t(%f)\n", pv_move_hits_prob, pv_move_searches, (float) pv_move_hits_prob/(float) pv_move_searches);
        // printf("|\n");
        printf("| Hits/Searched(Mean):\n| %d/%d\t(%f)\n", pv_move_hits_mean, pv_move_searches, (float)pv_move_hits_mean / (float)pv_move_searches);
        printf("================================\n\n\n");
    }

    // =================================================================== //
    // PV/HASH MOVE: While starting a new iteration, the most important    //
    // move ordering technique is to try PV-Moves first. A PV-Move is part //
    // of the principal variation and therefor a best move found in the    //
    // previous iteration of an iterative deepening framework.             //
    // =================================================================== //
    if (entry_found) {
        pv_move_searches++;
        for (int i = 1; i <= movelst.nr_elem; i++) {
            if (is_same_move(movelst.array[i], pv_move)) {
                if (i == idx_of_max_prob) pv_move_hits_prob++;
                if (i == idx_of_max_mean) pv_move_hits_mean++;
                movelst.array[i]->value = 10000;
                swap(&movelst, i, 1);
                break;
            }
        }
    }

    free_move(pv_move);

    int legal_moves = 0;
    int tt_flag = UPPERBOUND;
    int best_eval = NEGINF;
    move_t *best_move = NULL;

    while ((move = pop_max(&movelst))) {
        legal_moves++;

        do_move(searchdata->board, move);
        int eval = -negamax(searchdata, depth - 1, -beta, -best_eval);
        undo_move(searchdata->board, move);

        if (eval > best_eval) {
            best_eval = eval;
            free_move(best_move);
            best_move = copy_move(move);
        }
        free_move(move);

        // Alpha bound adjustment
        if (eval > alpha) {
            alpha = eval;
            tt_flag = EXACT;
        }

        // Beta cutoff
        if (alpha >= beta) {
            tt_flag = LOWERBOUND;
            break;
        }
    }
    free_pq(&movelst);

    // If the player had no legal moves, the game is over (atleast in this
    // branch of the search)
    if (legal_moves == 0) {
        free_move(best_move);

        // We wan't to determine if the player was check mated
        if (is_in_check(searchdata->board)) {
            return -16000 - depth;
        }
        // Or if we reached a stalemate
        else {
            return 0;
        }
    }

    // ================================================================ //
    // TRANSPOSITION TABLE STORING: We store the best move (the search  //
    // depth, it's evaluation and search flag) in the table. This       //
    // information can then be used if we encounter the same postion    //
    // again, and we don't need to search that branch any further. We   //
    // should be careful, to only store this information if the search  //
    // was not aborted by callee or due to time expiration (since we    //
    // cant be sure that the information is truely correct).            //
    // ================================================================ //
    if (!stop_immediately) {
        store_hashtable_entry(searchdata->board, tt_flag, best_eval, best_move,
                              depth);
    }
    free(best_move);

    return best_eval;
}

void search(searchdata_t *searchdata) {
    // Reset the history hash table from previous searches
    // Of course we keep the move_keys of already played
    // positions untouched
    for (int i = searchdata->board->ply_no; i < MAXPLIES; i++) {
        searchdata->board->history[i].hash = 0ULL;
    }

    // Reset the performance counters and calculate the time available for
    // search
    searchdata->best_eval = NEGINF;
    searchdata->best_move = NULL;
    searchdata->nodes_searched = 0;
    searchdata->hash_used = 0;
    searchdata->hash_bounds_adjusted = 0;
    searchdata->pv_node_hit = 0;
    searchdata->time_available = calculate_time(searchdata);

    int alpha = NEGINF, beta = INF;

    // =================================================================== //
    // ITERATIVE DEEPINING: It has been noticed, that even if one is about //
    // to search to a given depth, that iterative deepening is faster than //
    // searching for the given depth immediately. This is due to dynamic   //
    // move ordering techniques such as; PV- and hash- moves determined in //
    // previous iteration(s), as well the history heuristic (TODO).        //
    // =================================================================== //
    for (int depth = 1; depth <= searchdata->max_depth && depth < MAXDEPTH;
         depth++) {
        int eval = negamax(searchdata, depth, alpha, beta);

        if (stop_immediately) {
            stop_immediately = 0;
            if (searchdata->best_move == NULL && depth == 1) {
                searchdata->best_move =
                    get_best_move_from_hashtable(searchdata->board);
            }
            break;
        }

        // ================================================================ //
        // ASPIRATION WINDOWS: The technique is to use a guess of the       //
        // expected value (from the last iteration in iterative deepening)  //
        // and use a window around this as the alpha-beta bounds. Because   //
        // the window is narrower, more beta cutoffs are achieved, and the  //
        // search takes a shorter time. The drawback is that if the true    //
        // score is outside this window, then a costly re-search must be    //
        // made. Typical window sizes are 1/2 to 1/4 of a pawn on either    //
        // side of the guess.                                               //
        // ================================================================ //
        if (eval <= alpha || eval >= beta) {
            alpha = NEGINF;
            beta = INF;
            depth--;
            continue;
        }

        alpha = eval - WINDOWSIZE;
        beta = eval + WINDOWSIZE;

        // Update search data and output info (for GUI)
        free_move(searchdata->best_move);
        searchdata->best_move = get_best_move_from_hashtable(searchdata->board);
        searchdata->best_eval = eval;

        int nodes = searchdata->nodes_searched;
        int seldepth = searchdata->max_seldepth;
        int nps = (int)(nodes / delta_in_ms(searchdata));
        int time = delta_in_ms(searchdata);
        int hashfull = hashtable_full_permill();
        char *score = get_mate_or_cp_value(eval, depth);

        printf("info score %s depth %d seldepth %d nodes %d time %d nps %d hasfull %d pv ",
               score, depth, seldepth, nodes, time, nps, hashfull);
        print_line(searchdata->board, depth);
        printf("\n");
        if (eval >= 16000 || eval <= -16000) break;

        free(score);
    }

    int nodes = searchdata->nodes_searched;
    int nps = (int)(nodes / delta_in_ms(searchdata));
    int time = delta_in_ms(searchdata);
    int hashfull = hashtable_full_permill();
    char *move_str =
        get_LAN_move(searchdata->best_move, searchdata->board->player);
    printf("info nodes %d time %d nps %d hasfull %d\nbestmove %s\n", nodes,
           time, nps, hashfull, move_str);
    printf("\n");
    free(move_str);
}
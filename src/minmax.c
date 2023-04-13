#include "../include/chess.h"
#include "../include/eval.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h" 

#define TOLERANCE 10 //ms
#define ACCURACY 10000

int last_check = 0;
int stop_immediate = FALSE;

/* Determines how much time is available for search (search parameters specified by the caller (the gui)) */
int calculate_time(searchdata_t *data){
    int time_available = 0;
    int atleast_one_time_found = FALSE;
    int time_available_movetime = 0;
    int time_available_remainingtime = 0;

    // if max time for move is given 
    if(data->max_time != -1){
        // calculate time avaiable for search
        time_available_movetime += data->max_time;
        time_available_movetime = (data->board->player == WHITE)?(time_available_movetime+data->winc):(time_available_movetime+data->binc);
        time_available_movetime -= TOLERANCE;

        time_available = time_available_movetime;
        // make sure chess engine has atleast 5ms for search
        if(time_available < 5){
            time_available = 5;
        }
        atleast_one_time_found = TRUE;
    } 
    // if whites/blacks remaining time is given
    if ((data->board->player == WHITE && data->wtime != -1) || (data->board->player == BLACK && data->btime != -1)){
        // calculate time avaiable for search
        time_available_remainingtime = (data->board->player == WHITE)?((int)((double) data->wtime/30.0)):((int)((double) data->btime/30.0));
        time_available_remainingtime = (data->board->player == WHITE)?(time_available_remainingtime+data->winc):(time_available_remainingtime+data->binc);
        time_available_remainingtime -= TOLERANCE;

        // if no max time was given or the now calculated time is lower than the max time given update it
        if(!atleast_one_time_found || time_available_remainingtime < time_available_movetime){
            time_available = time_available_remainingtime;
            // again, make sure chess engine has atleast 5ms for search
            if(time_available < 5){
                time_available = 5;
            }
        }
        atleast_one_time_found = TRUE;
    }
    
    if(atleast_one_time_found){
        // return the minimum of both times calculated (or one if only one was given)
        return time_available;
    } else{
        // -1, indicates that no time limit was specified in the call
        return -1;
    }
    
}

/* Determines if the search has to be stopped */
/* Because of either (1) a STOP request or (2) we have used up our time to search */
int search_has_to_be_stopped(searchdata_t *search_data){
    // if a stop was initiaited, stop the search immediately
    if(search_data->stop){
        return 1;
    }
    // or, if search is not in infinite mode and the time has run out, stop search immediately
    if(!search_data->run_infinite){
        if((int) (((double) clock() - search_data->start_time) / 1000) >= search_data->time_available){
            return 1;
        }
    }
    return 0;
}

/* Quescience search */
int quiet_search(board_t *board, int alpha, int beta, searchdata_t* search_data){
    int eval = eval_board(board);

    if(eval >= beta){
        return eval;
    }
    if(eval > alpha){
        alpha = eval;
    }

    int best_eval = eval;

    // generate only pseudo legal moves
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_pseudo_moves(board, &movelst);
    move_t* move;

    while((move = pop_max(&movelst))){
        // every so often check if we have to stop the search
        if(last_check > ACCURACY && !stop_immediate){
            last_check = 0;
            stop_immediate = search_has_to_be_stopped(search_data);
        }

        // exit search cleanly if we have to stop 
        if(stop_immediate){
            free_move(move);
            free_pq(&movelst);
            return get_eval_from_hashtable(board);
        }

        // filter out non-captures
        if(!(move->flags &0b0100)){
            free_move(move);
            continue;
        }

        
        search_data->nodes_searched++;
        last_check++;

        // delta pruning
        if(best_eval - 200 - is_capture(move->to, board) > eval){
            free_move(move);
            continue;
        }


        do_move(board, move);
        eval = -quiet_search(board, -beta, -alpha, search_data);
        undo_move(board);

        // if eval is better than the best so far, update it 
        if(eval > best_eval){
            best_eval = eval;
        }
        // if eval is better than alpha, adjust bound 
        if(eval > alpha){
            alpha = eval;
        }
        // beta-cutoff 
        if(eval >= beta){
            free_move(move);
            break;
        }
        free_move(move);
    }

    free_pq(&movelst);
        
    return best_eval;
}

/* Main entry point of search */
int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* search_data){
    int old_alpha = alpha;
    int old_beta = beta;
    int best_eval = NEGINFINITY;
    move_t* best_move = NULL;

    move_t *pv_move = NULL;
    int16_t pv_value;
    int8_t pv_flags;
    int8_t pv_depth;

    int entry_found = get_hashtable_entry(board, &pv_flags, &pv_value, &pv_move, &pv_depth);
    
    // (1) FIRSTLY, if there exists a TT entry for the board, use the information we stored 
    // HINT: There will never exist an entry for depth 0 -> if depth 0 this part will simply be skipped
    if (entry_found && pv_depth == depth) {
        // if eval of pvmove is exact we can use it as the best move so far
        if (pv_flags == EXACT) {
            search_data->nodes_searched++;
            search_data->hash_used++;
            free_move(pv_move);
            return pv_value;
        }
        // if the pvmove caused a beta cutoff last time
        // we can use its eval as a lower(=alpha) bound 
        else if (pv_flags == LOWERBOUND) {
            search_data->hash_bounds_adjusted++;
            if(pv_value > alpha) alpha = pv_value;
        } 
        // if no move was better than the pvmove last time
        // we can use its eval as an upper(=beta) bound
        else if (pv_flags == UPPERBOUND) {
            search_data->hash_bounds_adjusted++;
            if(pv_value < beta) beta = pv_value;
        }
        // beta cutoff
        if (alpha >= beta) {
            search_data->nodes_searched++;
            search_data->hash_used++;
            free_move(pv_move);
            return pv_value;
        }
    }

    // (2) SECONDLY, if we have reached depth 0
    if(depth == 0){
        // start quescience search 
        return quiet_search(board, alpha, beta, search_data);
    }

    //
    // NOW THE "ACTUAL" SEARCH BEGINS 
    //

    // (3) NOW, search the pvmove first (since its likely the best) and will cause the most cutoffs
    if(entry_found){
        // every so often check if we have to stop the search
        if(last_check > ACCURACY && !stop_immediate){
            last_check = 0;
            stop_immediate = search_has_to_be_stopped(search_data);
        }

        // exit search cleanly if we have to stop 
        if(stop_immediate){
            free_move(pv_move);
            free_move(best_move);
            return (entry_found) ? pv_value : -16000;
        }
        
        search_data->nodes_searched++;
        last_check++;

        do_move(board, pv_move);
        int eval = -alpha_beta_search(board, depth-1, -beta, -best_eval, search_data);
        undo_move(board);

        // adjust best eval and best move 
        best_eval = eval;
        free_move(best_move);
        best_move = copy_move(pv_move);

        // if eval is better than alpha, adjust bound
        if(eval > alpha){
            alpha = eval;
        }

        // beta cutoff
        if(eval >= beta){
            search_data->pv_node_hit++;
            free_move(pv_move);
            free_move(best_move);
            return eval;
        }
    }

    // (4) NOW, continue to search all the other moves 

    // generate all (pseudo)legal moves 
    maxpq_t movelst;
    move_t* move;
    initialize_maxpq(&movelst);
    generate_pseudo_moves(board, &movelst);

    // keep track of how many legal moves encountered
    int nr_legal_moves = 0;

    while((move = pop_max(&movelst))){
        // every so often check if we have to stop the search
        if(last_check > ACCURACY && !stop_immediate){
            last_check = 0;
            stop_immediate = search_has_to_be_stopped(search_data);
        }
        
        // exit search cleanly if we have to stop 
        if(stop_immediate){
            free_move(pv_move);
            free_move(move);
            free_move(best_move);
            free_pq(&movelst);
            return (entry_found) ? pv_value : -16000;
        }
        last_check++;

        // filter out pv move (since we checked it already)
        if(entry_found && is_same_move(move, pv_move)){
            nr_legal_moves++;
            free_move(move);
            continue;
        }

        do_move(board, move);

        // filter out illegal moves
        if(is_in_check_after_move(board)){
            undo_move(board);
            free_move(move);
            continue;
        }

        nr_legal_moves++;
        search_data->nodes_searched++;

        // if the move was legal move, continue the search 
        int eval = -alpha_beta_search(board, depth-1, -beta, -best_eval, search_data);
        undo_move(board);

        // if eval is better than the best so far update it 
        if(eval > best_eval){
            best_eval = eval;
            free(best_move);
            best_move = copy_move(move);
        }
        // if eval is better than alpha, adjust bound 
        if(eval > alpha){
            alpha = eval;
        }

        // beta cutoff 
        if(eval >= beta){
            free_move(move);
            break;
        }

        free_move(move);
    }

    // if player had no legal moves 
    if(nr_legal_moves == 0){
        // game over (atleast in this branch of the search) 
        free_move(pv_move);
        free(best_move);
        // we need to determine if stalemate or check
        return eval_end_of_game(board, depth);
    }

    // if we exited early due to cutoff we cant store eval as a lower bound
    if(best_eval >= old_beta){
        store_hashtable_entry(board, LOWERBOUND, best_eval, best_move, depth);
    }
    // if no move was better than alpha than we can store the best evaluation as a upperbound 
    else if(best_eval <= old_alpha){
        store_hashtable_entry(board, UPPERBOUND, best_eval, best_move, depth);
    } 
    // else we can store the eval as an exact evaluation of the current board 
    else{
        store_hashtable_entry(board, EXACT, best_eval, best_move, depth);
    }
    
    free_move(pv_move);
    free(best_move);
    free_pq(&movelst);

    return best_eval;
}

int iterative_search(searchdata_t* search_data) {
    // if there is a max depth given, use it, else search up to 100 plies (some arbitrary search depth we will never reach)
    int maxdepth = (search_data->max_depth == -1)?100:search_data->max_depth;

    // reset the performance counters 
    search_data->nodes_searched = 0;
    search_data->hash_used = 0;
    search_data->hash_bounds_adjusted = 0;
    search_data->pv_node_hit = 0;

    // reset search variables 
    int evaluation = 0;
    move_t *best_move = NULL;
    search_data->time_available = calculate_time(search_data);

    // iterative search 
    for (int i = 1; i <= maxdepth; i++) {
        search_data->current_max_depth = i;
        int current_evaluation;

        // aspiration windows
        if(i == 1){
            current_evaluation = alpha_beta_search(search_data->board, i, NEGINFINITY, INFINITY, search_data);        
        } else{
            int window_low = evaluation - 50;
            int window_high = evaluation + 50;
            // use last searchs evaluation as a good estimate of the next and use smaller windows 
            current_evaluation = alpha_beta_search(search_data->board, i, window_low, window_high, search_data); 
            // if our result lies outside our window, we (unfortunately) have to do a costly research with a bigger window
            if(current_evaluation > window_high) current_evaluation = alpha_beta_search(search_data->board, i, window_low, INFINITY, search_data); 
            else if(current_evaluation < window_low) current_evaluation = alpha_beta_search(search_data->board, i, NEGINFINITY, window_high, search_data); 
        }
        
        // update best move and its evaluation
        if(best_move) free_move(best_move);
        best_move = get_best_move_from_hashtable(search_data->board);
        evaluation = current_evaluation;

        // output search info (for the gui)
        if(current_evaluation > 16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", (int) i/2, i, search_data->nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) search_data->nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))));
            print_line(search_data->board, i);
            printf("\n");
            break;
        } else if (current_evaluation < -16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", -1*((int) i/2), i, search_data->nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) search_data->nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))));
            print_line(search_data->board, i);
            printf("\n");
            break;
        } else{
            printf("info score cp %d depth %d nodes %d time %d nps %d hashfull %d pv ", current_evaluation, i, search_data->nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) search_data->nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))), hashtable_full_permill());
            print_line(search_data->board, i);
            printf("\n");
        }

        // exit search if we have to
        if(stop_immediate){
            stop_immediate = FALSE;
            break;
        }
        
    }

    search_data->best_move = best_move;

    return evaluation;
}
#include "../include/chess.h"
#include "../include/eval.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h" 

int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* search_data){
    if(depth == 0){
        return eval_board(board);
    }

    int old_alpha = alpha;
    int old_beta = beta;
    int best_eval = NEGINFINITY;
    move_t* best_move = NULL;

    move_t *pv_move = NULL;
    int16_t pv_value;
    int8_t pv_flags;
    int8_t pv_depth;

    int entry_found = get_hashtable_entry(board, &pv_flags, &pv_value, &pv_move, &pv_depth);

    if (entry_found && pv_depth == depth) {
        /* if eval of pvmove is exact we can use it as the best move so far */
        if (pv_flags == EXACT) {
            hash_used++;
            free_move(pv_move);
            return pv_value;
        }
        else if (pv_flags == LOWERBOUND) {
            hash_bounds_adjusted++;
            if(pv_value > alpha) alpha = pv_value;
        } 
        else if (pv_flags == UPPERBOUND) {
            hash_bounds_adjusted++;
            if(pv_value < beta) beta = pv_value;
        }
        if (alpha >= beta) {
            hash_used++;
            free_move(pv_move);
            return pv_value;
        }
    }

    /* search pv move first */
    if(entry_found){
        do_move(board, pv_move);

        nodes_searched++;

        int eval = -alpha_beta_search(board, depth-1, -beta, -best_eval, search_data);

        best_eval = eval;
        free_move(best_move);
        best_move = copy_move(pv_move);

        /* if eval is better than alpha, adjust bound */
        if(eval > alpha){
            alpha = eval;
        }

        undo_move(board);

        /* beta cutoff */
        if(eval >= beta){
            pv_node_hit++;
            free_move(pv_move);
            free_move(best_move);
            return eval;
        }
        free_move(pv_move);
    }

    /* generate only pseudo legal moves */
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    
    generate_pseudo_moves(board, &movelst);
    int nr_legal_moves = 0;
    move_t* move;

    while((move = pop_max(&movelst))){
        do_move(board, move);

        /* filter out illegal moves */
        if(is_in_check_after_move(board)){
            undo_move(board);
            free_move(move);
            continue;
        }
        nr_legal_moves++;
        nodes_searched++;

        /* if legal move -> continue search */
        int eval = -alpha_beta_search(board, depth-1, -beta, -best_eval, search_data);

        /* if eval is better than the best so far */
        if(eval > best_eval){
            best_eval = eval;
            free(best_move);
            best_move = copy_move(move);
        }
        /* if eval is better than alpha, adjust bound */
        if(eval > alpha){
            alpha = eval;
        }

        undo_move(board);

        /* if beta cutoff */
        if(eval >= beta){
            free_move(move);
            break;
        }

        free_move(move);
    }

    /* if player had no legal moves */
    if(!nr_legal_moves){
        /* game over (in this branch) */
        free(best_move);
        return eval_end_of_game(board, depth);
    }

    /* if we exited early due to cutoff we cant store eval as exact */
    if(best_eval >= old_beta){
        store_hashtable_entry(board, LOWERBOUND, best_eval, best_move, depth);
    }
    /* if no move was better than alpha than we can store the best evaluation as a upperbound */
    else if(best_eval <= old_alpha){
        store_hashtable_entry(board, UPPERBOUND, best_eval, best_move, depth);
    } 
    /* else we can store the eval as an exact evaluation of the current board */
    else{
        store_hashtable_entry(board, EXACT, best_eval, best_move, depth);
    }
    
    free(best_move);
    free_pq(&movelst);

    return best_eval;
}

int iterative_search(searchdata_t* search_data) {
    /* if max depth given use it, else search up to 100 plies */
    int maxdepth = (search_data->max_depth == -1)?100:search_data->max_depth;
    /* variables to keep track of best move and evaluation */
    move_t *best_move = NULL;
    int evaluation = 0;

    /* iterative search */
    for (int i = 1; i <= maxdepth; i++) {
        int start_search = clock();
        /* reset the performance counters */
        nodes_searched = 0;
        pv_node_hit = 0;

        search_data->current_max_depth = i;
        int current_evaluation = alpha_beta_search(search_data->board, i, NEGINFINITY, INFINITY, search_data);

        if(best_move) free_move(best_move);
        best_move = get_best_move_from_hashtable(search_data->board);
        evaluation = current_evaluation;

        if(current_evaluation > 16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", (int) i/2, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC), (int) (((double) nodes_searched)/(((double)(clock() - (start_search)) / CLOCKS_PER_SEC))));
            print_LAN_move(best_move, search_data->board->player);
            printf("\n");
            goto search_finished;
        } else if (current_evaluation < -16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", -1*((int) i/2), i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) nodes_searched)/(((double)(clock() - (start_search)) / CLOCKS_PER_SEC))));
            print_LAN_move(best_move, search_data->board->player);
            printf("\n");
            goto search_finished;
        } else{
            printf("info score cp %d depth %d nodes %d time %d nps %d hashfull %d pv ", current_evaluation, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) nodes_searched)/(((double)(clock() - (start_search)) / CLOCKS_PER_SEC))), hashtable_full_permill());
            print_line(search_data->board, i);
            printf("\n");
            fprintf(stderr, "Hashtable size: %.2f MBytes\t (%.1f%%)\n", (float) ((float) get_memory_usage_hashtable_in_bytes())/(1024.0*1024.0), ((float) hashtable_full_permill())/10.0);
        }
    }

    search_finished:

    search_data->best_move = best_move;

    return evaluation;
}
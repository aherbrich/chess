#include "../include/chess.h"
#include "../include/eval.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h" 

int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* search_data){
    if(depth == 0){
        return eval_board(board);
    }

    int max_value = alpha;

    move_t* pvmove;
    if((pvmove = get_best_move_from_hashtable(board))){
        do_move(board, pvmove);
        nodes_searched++;
        int eval = -alpha_beta_search(board, depth-1, -beta, -max_value, search_data);
        undo_move(board);

        if(eval > max_value){
            max_value = eval;
            if(depth == search_data->current_max_depth){
                store_hashtable_entry(board, 0, max_value, pvmove, depth);
            }
            if(max_value >= beta){
                free_move(pvmove);
                return max_value;
            }
        }
    }

    /* generate only pseudo legal moves */
    list_t* movelst = generate_pseudo_moves(board);
    int nr_legal_moves = 0;

    while(movelst->len != 0){
        move_t *move = pop(movelst);

        /* filter out pvmove (since we already checked it) */
        if(pvmove && is_same_move(pvmove, move)){
            free(move);
            continue;
        }   

        do_move(board, move);

        /* filter out illegal moves */
        if(is_in_check_after_move(board)){
            undo_move(board);
            free(move);
            continue;
        }
        nr_legal_moves++;
        nodes_searched++;

        /* if legal move -> continue search */
        int eval = -alpha_beta_search(board, depth-1, -beta, -max_value, search_data);
        undo_move(board);

        if(eval > max_value){
            max_value = eval;
            if(depth == search_data->current_max_depth){
                store_hashtable_entry(board, 0, max_value, move, depth);
            }
            if(max_value >= beta){
                free_move(pvmove);
                free_move(move);
                break;
            }
        }
        free_move(move);
    }
    free_move_list(movelst);

    /* if player had no legal moves */
    if(!nr_legal_moves){
        /* game over (in this branch) */
        return eval_end_of_game(board, depth);
    }

    return max_value;
}

int iterative_search(searchdata_t* search_data) {
    /* reset the performance counters */
    nodes_searched = 0;

    /* if max depth given use it, else search up to 100 plies */
    int maxdepth = (search_data->max_depth == -1)?100:search_data->max_depth;

    /* variables to keep track of best move and evaluation */
    move_t *best_move = NULL;
    int evaluation = 0;

    /* iterative search */
    for (int i = 1; i <= maxdepth; i++) {
        search_data->current_max_depth = i;
        int current_evaluation = alpha_beta_search(search_data->board, i, NEGINFINITY, INFINITY, search_data);

        best_move = get_best_move_from_hashtable(search_data->board);
        evaluation = current_evaluation;

        if(current_evaluation > 16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", (int) i/2, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))));
            print_LAN_move(best_move, search_data->board->player);
            printf("\n");
            goto search_finished;
        } else if (current_evaluation < -16000){
            printf("info score mate %d depth %d nodes %d time %d nps %d pv ", -1*((int) i/2), i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))));
            print_LAN_move(best_move, search_data->board->player);
            printf("\n");
            goto search_finished;
        } else{
            printf("info score cp %d depth %d nodes %d time %d nps %d pv ", current_evaluation, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000), (int) (((double) nodes_searched)/(((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC))));
            print_LAN_move(best_move, search_data->board->player);
            printf("\n");
        }
    }

    search_finished:

    search_data->best_move = best_move;

    return evaluation;
}
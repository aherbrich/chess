#include "../include/chess.h"

int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* search_data){
    if(depth == 0){
        nodes_searched++;
        return eval_board(board);
    }

    int max_value = alpha;

    /* generate only pseudo legal moves */
    list_t* movelst = generate_pseudo_moves(board);
    int nr_legal_moves = 0;

    while(movelst->len != 0){
        move_t *move = pop(movelst);

        do_move(board, move);

        /* filter out illegal moves */
        if(is_in_check_after_move(board)){
            undo_move(board);
            free(move);
            continue;
        }
        nr_legal_moves++;

        /* if legal continue as usual */
        int eval = -alpha_beta_search(board, depth-1, -beta, -max_value, search_data);
        undo_move(board);

        if(eval > max_value){
            max_value = eval;
            if(depth == search_data->max_depth){
                if(search_data->best_move) free(search_data->best_move);
                search_data->best_move = copy_move(move);
            }
            if(max_value >= beta){
                free_move(move);
                break;
            }
        }
        free_move(move);
    }
    free_move_list(movelst);

    if(!nr_legal_moves){
        nodes_searched++;
        return eval_end_of_game(board, depth);
    }

    return max_value;
}

int iterative_search(searchdata_t* search_data) {
    /* reset the performance counters */
    nodes_searched = 0;

    int maxdepth = (search_data->max_depth == -1)?100:search_data->max_depth;
    move_t *best_move = NULL;
    int best_eval = 0;

    for (int i = 1; i <= maxdepth; i++) {
        int evaluation = alpha_beta_search(search_data->board, i, NEGINFINITY, INFINITY, search_data);

        if(best_move) free_move(best_move);
        best_move = search_data->best_move;
        best_eval = evaluation;

        if(evaluation > 16000){
            printf("info score mate %d depth %d nodes %d time %d", (int) i/2, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000));
            printf("\n");
            goto search_finished;
        } else if (evaluation < -16000){
            printf("info score mate %d depth %d nodes %d time %d", -1*((int) i/2), i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000));
            printf("\n");
            goto search_finished;
        } else{
            printf("info score cp %d depth %d nodes %d time %d", evaluation, i, nodes_searched, (int) ((double)(clock() - (search_data->start_time)) / CLOCKS_PER_SEC*1000));
            printf("\n");
        }
    }

    search_finished:

    return best_eval;
}
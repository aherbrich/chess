#include "../include/chess.h"

int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* search_data){
    list_t* movelst = generate_moves(board);

    if(movelst->len == 0){
        nodes_searched++;
        return eval_end_of_game(board, depth);
    }

    if(depth == 0){
        free_move_list(movelst);
        nodes_searched++;
        return eval_board(board);
    }

    int max_value = alpha;

    while(movelst->len != 0){
        move_t *move = pop(movelst);

        do_move(board, move);
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

    return max_value;
}
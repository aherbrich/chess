#include "../include/chess.h"

/* Initializes search data structure */
searchdata_t *init_search_data(board_t *board) {
    searchdata_t *data = (searchdata_t *)malloc(sizeof(searchdata_t));

    data->board = copy_board(board);  // current board
    data->max_depth = -1;             // maximum search depth in plies
    data->max_seldepth = -1;  // maximum search depth with quiescence search
    data->max_nodes = -1;     // maximum nodes allowed to search
    data->max_time = -1;      // maximum time allowed in ms
    data->wtime = -1;         // white time increment in ms
    data->btime = -1;         // black time increment in ms
    data->winc = 0;           // white time increment in ms
    data->binc = 0;           // black time increment in ms
    data->ponder = 0;         // tells engine to start search at ponder move
    data->run_infinite = 0;   // tells the engine to run aslong as stop != 1
    data->stop = 0;           // tells the engine to stop search when stop == 1
    data->time_available =
        0;  // tells the engine how much time it has to search in ms

    gettimeofday(&(data->start), 0);
    data->best_move = NULL;         // best move (so far)
    data->best_eval = NEGINFINITY;  // evaluation of board after best move made
    data->nodes_searched = 0;  // amount of nodes searched in iterative search
    data->hash_used = 0;  // amount of hash entries that lead to not needing to
                          // search the node again
    data->hash_bounds_adjusted = 0;  // amount of hash entries that lead to a
                                     // adjustment of alpha/beta bounds
    data->pv_node_hit =
        0;  // amount of pv moves that turned out to be the best move
    return data;
}

/* Frees search data structure */
void free_search_data(searchdata_t *data) {
    free(data->board);
    free(data->best_move);
    free(data);
}
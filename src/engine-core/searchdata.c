#include <sys/time.h>

#include "include/engine-core/search.h"

#include "include/engine-core/types.h"
#include "include/engine-core/tt.h"
#include "include/engine-core/board.h"
#include "include/engine-core/move.h"

/* Initializes search data structure */
searchdata_t *init_search_data(board_t *board) {
    searchdata_t *data = (searchdata_t *)malloc(sizeof(searchdata_t));

    data->board = copy_board(board);  // current board
    data->tt = init_tt(MB_TO_BYTES(64));  // transposition table
    data->max_depth = 100;             // maximum search depth in plies
    data->max_nodes = -1;             // maximum nodes allowed to search
    data->max_time = -1;              // maximum time allowed in ms
    data->wtime = -1;                 // white time increment in ms
    data->btime = -1;                 // black time increment in ms
    data->winc = 0;                   // white time increment in ms
    data->binc = 0;                   // black time increment in ms
    data->ponder = 0;                 // tells engine to start search at ponder move
    data->run_infinite = 1;           // tells the engine to run aslong as stop != 1
    data->stop = 0;                   // tells the engine to stop search when stop == 1
    data->time_available = 0;  // tells the engine how much time it has to search in ms

    gettimeofday(&(data->start), 0);
    data->max_seldepth = -1;          // maximum search depth with quiescence search
    data->best_move = NULL;           // best move in (iterative) search so far
    data->best_eval = NEGINF;        // evaluation of board after best move made
    data->nodes_searched = 0;        // amount of nodes searched in iterative search
    data->hash_used = 0;             // amount of hash entries that lead to not needing to
                                     // search the node again
    data->hash_bounds_adjusted = 0;  // amount of hash entries that lead to a
                                     // adjustment of alpha/beta bounds
    data->pv_node_hit =
        0;  // amount of pv moves that turned out to be the best move
    return data;
}

/* Frees search data structure */
void free_search_data(searchdata_t *data) {
    free_tt(data->tt);
    free_move(data->best_move);
    free(data->board);
    free(data);
}

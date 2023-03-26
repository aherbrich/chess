#include "../include/chess.h"

/* initializes search data structure */
searchdata_t* init_search_data(board_t *board){
    searchdata_t *data = (searchdata_t*) malloc(sizeof(searchdata_t));
    data->board = copy_board(board);  /* current board */
    data->max_depth = -1;             /* maximum search depth in plies */
    data->current_max_depth = -1;     /* current maximum search depth in plies in iterative search*/
    data->max_seldepth = -1;          /* maximum search depth with quiescence search */
    data->max_nodes = -1;             /* maximum nodes allowed to search */
    data->max_time = -1;              /* maximum time allowed in ms */
    data->wtime = -1;                 /* white time increment in ms */
    data->btime = -1;                 /* black time increment in ms */
    data->winc = 0;                   /* white time increment in ms */      
    data->binc = 0;                   /* black time increment in ms */ 
    data->ponder = 0;                 /* tells engine to start search at ponder move */
    data->run_infinite = 0;           /* tells the engine to run aslong as stop != 1 */
    data->stop = 0;                   /* tells the engine to stop search when stop == 1*/
    data->best_move = NULL;           /* computed best move (so far) */
    data->start_time = clock();       /* time the search was initiated by the gui */
    data->time_available = 0;
    return data;
}

/* frees search data structure */
void free_search_data(searchdata_t *data){
    free(data->board);          
    free(data->best_move);
    free(data);
}
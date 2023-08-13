#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <limits.h>

#include "include/engine-core/types.h"

#define INF INT_MAX
#define NEGINF (-INF)

typedef enum _ttflag_t {
    EXACT,
    UPPERBOUND,
    LOWERBOUND
} ttflag_t;

typedef struct _searchdata_t {
    board_t* board;      // pointer to the actual board
    int max_depth;       // maximum search depth in plies
    int max_seldepth;    // maximum search depth with quiescence search
    int max_nodes;       // maximum nodes allowed to search
    int max_time;        // maximum time allowed
    int wtime;           // time white has left on clock in ms
    int btime;           // time black has left on clock in ms
    int winc;            // white time increment in ms
    int binc;            // black time increment in ms
    int ponder;          // tells engine to start search at ponder move
    int run_infinite;    // tells the engine to run aslong as stop != 1
    int stop;            // tells the engine to stop search when stop == 1
    int time_available;  // tells the engine how much time it has to search in
                         // ms

    struct timeval start;
    struct timeval end;
    move_t* best_move;         // best move (so far)
    int best_eval;             // evaluation of board after best move made
    int nodes_searched;        // amount of nodes searched in iterative search
    int hash_used;             // amount of hash entries that lead to not needing to search
                               // the node again
    int hash_bounds_adjusted;  // amount of hash entries that lead to a
                               // adjustment of alpha/beta bounds
    int pv_node_hit;           // amount of pv moves that turned out to be the best move
} searchdata_t;

///////////////////////////////////////////////////////////////
//  SEARCH DATA
searchdata_t* init_search_data(board_t* board);
void free_search_data(searchdata_t* data);

///////////////////////////////////////////////////////////////
//  SEARCH
void search(searchdata_t* search_data);

#endif
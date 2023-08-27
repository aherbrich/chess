#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <limits.h>

#include "include/engine-core/types.h"
#include "include/engine-core/tt.h"

#define INF INT_MAX
#define NEGINF (-INF)

typedef enum _ttflag_t {
    EXACT,
    UPPERBOUND,
    LOWERBOUND
} ttflag_t;

/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for managing the searchdata (time constraints, search info etc. )          */
/* ------------------------------------------------------------------------------------------------ */
typedef struct _searchdata_t {
    board_t* board;                 /* pointer to the actual board */
    tt_t tt;                        /* transposition table for the search */

    int max_depth;                  /* maximum search depth in plies */
    int max_seldepth;               /* maximum search depth with quiescence search */
    int max_nodes;                  /* maximum nodes allowed to search */
    int max_time;                   /* maximum time allowed */
    int wtime;                      /* time white has left on clock in ms */
    int btime;                      /* time black has left on clock in ms */
    int winc;                       /* white time increment in ms */
    int binc;                       /* black time increment in ms */
    int ponder;                     /* tells engine to start search at ponder move */
    int run_infinite;               /* tells the engine to run aslong as stop != 1 */
    int stop;                       /* tells the engine to stop search when stop == 1 */
    int time_available;             /* tells the engine how much time it has to search in ms */

    struct timeval start;           /* start time of search (μs accuracy) */
    struct timeval end;             /* end time of search (μs accuracy) */

    move_t* best_move;              /* best move in (iterative) search so far */    
    int best_eval;                  /* corresponding evaluation of best move */
    int nodes_searched;             /* amount of nodes searched */
    int hash_used;                  /* amount of hash table hits that lead to not 
                                       needing to search the node again */
    int hash_bounds_adjusted;       /* amount of hash table hits that lead to not 
                                       adjustment of alpha/beta bounds */
    int pv_node_hit;                /* amount of pv moves that turned out to be the best move */
} searchdata_t;

/* returns an initialized searchdata struct with default values */
searchdata_t* init_search_data(board_t* board);
/* frees memory for searchdata struct */
void free_search_data(searchdata_t* data);

/* ------------------------------------------------------------------------------------------------ */
/* functions concerning search                                                                      */
/* ------------------------------------------------------------------------------------------------ */

/* starts the search */
void search(searchdata_t* search_data);

#endif

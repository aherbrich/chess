#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <limits.h>

#include "include/engine-core/types.h"
#include "include/engine-core/tt.h"

#define INF INT_MAX
#define NEGINF (-INF)

#define MAXDEPTH 100    // plies
#define STOP_ACCURACY 255 // nodes
#define WINDOWSIZE 50   // centipawns

typedef enum _ttflag_t {
    EXACT,
    UPPERBOUND,
    LOWERBOUND
} ttflag_t;

/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for managing the searchdata (time constraints, search info etc. )          */
/* ------------------------------------------------------------------------------------------------ */

typedef struct _timer_t {
    struct timeval start;           /* start time of search (μs accuracy) */

    int run_infinite;               /* tells the engine to run aslong as stop != 1 */
    int max_depth;                  /* maximum search depth in plies allowed to search */
    uint64_t max_nodes;             /* maximum nodes allowed to search */
    int stop;                       /* tells the engine to stop search when stop == 1 */

    int max_time;                   /* maximum time allowed */
    int wtime;                      /* time white has left on clock in ms */
    int btime;                      /* time black has left on clock in ms */
    int winc;                       /* white time increment in ms */
    int binc;                       /* black time increment in ms */

    int local_lag;                   /* time in ms subtracted from available time
                                        to compensate for lag on local machine */
    int remote_lag;                  /* time in ms subtracted from available time
                                        to compensate for lag of remote connection */
    int time_available;              /* tells the engine how much time it has to search in ms */
} timer_t;

typedef struct _searchdata_t {
    board_t* board;                 /* pointer to the actual board */
    tt_t tt;                        /* transposition table for the search */
    
    timer_t timer;                  /* timer for time management */

    int ponder;                     /* tells engine to start search at ponder move */

    int max_seldepth;               /* maximum depth searched while in quiescence search */
    move_t* best_move;              /* best move in (iterative) search so far */    
    int best_eval;                  /* corresponding evaluation of best move */
    uint64_t nodes_searched;             /* amount of nodes searched */
    int hash_used;                  /* amount of hash table hits that lead to not */
                                    /* needing to search the node again */
    int hash_bounds_adjusted;       /* amount of hash table hits that lead to */ 
                                    /* adjustment of alpha/beta bounds */
    int pv_node_hit;                /* amount of pv moves that turned out to be the best move */
} searchdata_t;

/* returns an initialized searchdata struct with default values */
searchdata_t* init_search_data(board_t* board, int tt_size_in_mb);
/* frees memory for searchdata struct */
void free_search_data(searchdata_t* data);

/* ------------------------------------------------------------------------------------------------ */
/* functions for time management                                                                    */
/* ------------------------------------------------------------------------------------------------ */

/* returns time passed while search in ms */
int delta_in_ms(searchdata_t *searchdata);
/* determines how much time is available for search (search parameters specified by the caller (the gui)) */
int calculate_time(searchdata_t *data);
/* determines if the search has to be stopped */
/* because of either (1) a STOP request or (2) we have used up our time to search */
void check_time(timer_t* timer);

/* ------------------------------------------------------------------------------------------------ */
/* functions concerning search                                                                      */
/* ------------------------------------------------------------------------------------------------ */

/* starts the search */
void search(searchdata_t* search_data);

#endif

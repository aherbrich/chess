#include <sys/time.h>

#include "include/engine-core/search.h"

#include "include/engine-core/types.h"
#include "include/engine-core/tt.h"
#include "include/engine-core/board.h"
#include "include/engine-core/move.h"

/* returns time passed while search in ms */
int delta_in_ms(searchdata_t *searchdata) {
    /* get current time */
    struct timeval now;
    gettimeofday(&now, 0);

    /* calculate time passed since start of search */
    int time_passed = (now.tv_sec - searchdata->timer.start.tv_sec) * 1000 +
                      (now.tv_usec - searchdata->timer.start.tv_usec) / 1000;
    
    return time_passed;
}

/* determines how much time is available for search (search parameters specified
 * by the caller (the gui)) */
int calculate_time(searchdata_t* data) {
    int time_available = 0;
    int atleast_one_time_found = 0;
    int time_available_movetime = 0;
    int time_available_remainingtime = 0;

    /* if max time for move is given */
    if (data->timer.max_time != -1) {
        /* calculate time avaiable for search */
        time_available_movetime += data->timer.max_time;
        time_available_movetime = (data->board->player == WHITE)
                                      ? (time_available_movetime + data-> timer.winc)
                                      : (time_available_movetime + data-> timer.binc);
        time_available_movetime -= TOLERANCE;

        time_available = time_available_movetime;
        /* make sure chess engine has atleast 5ms for search */
        if (time_available < 5) {
            time_available = 5;
        }
        atleast_one_time_found = 1;
    }
    /* if whites/blacks remaining time is given */
    if ((data->board->player == WHITE && data-> timer.wtime != -1) ||
        (data->board->player == BLACK && data-> timer.btime != -1)) {
        /* calculate time avaiable for search */
        time_available_remainingtime =
            (data->board->player == WHITE)
                ? ((int)((double)data-> timer.wtime / 30.0))
                : ((int)((double)data-> timer.btime / 30.0));
        time_available_remainingtime =
            (data->board->player == WHITE)
                ? (time_available_remainingtime + data-> timer.winc)
                : (time_available_remainingtime + data-> timer.binc);
        time_available_remainingtime -= TOLERANCE;

        /* if no max time was given or the now calculated time is lower than the */
        /* max time given update it */
        if (!atleast_one_time_found ||
            time_available_remainingtime < time_available_movetime) {
            time_available = time_available_remainingtime;
            /* again, make sure chess engine has atleast 5ms for search */
            if (time_available < 5) {
                time_available = 5;
            }
        }
        atleast_one_time_found = 1;
    }

    if (atleast_one_time_found) {
        /* return the minimum of both times calculated (or one if only one was given) */
        return time_available;
    } else {
        /* -1, indicates that no time limit was specified in the call */
        return -1;
    }
}


/* returns 1 if time is left, else 0 */
int time_left(struct timeval start, int time_available) {
    /* get current time */
    struct timeval now;
    gettimeofday(&now, 0);

    /* calculate time passed since start of search */
    int time_passed = (now.tv_sec - start.tv_sec) * 1000 +
                      (now.tv_usec - start.tv_usec) / 1000;

    /* calculate time left */
    return (time_available <= time_passed) ? 0 : 1;
}

/* determines if the search has to be stopped */
/* because of either (1) a STOP request or (2) we have used up our time to
 * search */
int search_has_to_be_stopped(timer_t timer) {
    /* if a stop was initiaited, stop the search immediately */
    if (timer.stop) {
        return 1;
    }
    /* or, if search is not in infinite mode and the time has run out, stop search immediately */
    if (!timer.run_infinite) {
        if (!time_left(timer.start, timer.time_available)) {
            return 1;
        }
    }
    return 0;
}

/* initializes timer */
timer_t init_timer(void) {
    timer_t timer;                                  /* timer for time management */

    gettimeofday(&(timer.start), 0);               /* start time of search */

    timer.run_infinite = 1;                        /* tells the engine to run aslong as stop != 1 */ 
    timer.max_depth = 100;                         /* maximum depth the engine will try to reach */
    timer.max_nodes = 18446744073709551615ULL;        /* initialized to uint64_max - maximum number of nodes allowed so search */
    timer.stop = 0;                                /* tells the engine to stop search when stop == 1 */

    timer.max_time = -1;                           /* maximum time allowed to search in ms */
    timer.wtime = -1;                              /* time white has left on clock in ms */
    timer.btime = -1;                              /* time black has left on clock in ms */
    timer.winc = -1;                               /* white time increment in ms */  
    timer.binc = -1;                               /* black time increment in ms */
    
    timer.time_available = -1;                     /* calculated once at the start of the search */
                                                   /* tells engine how much time (in ms) it has to search */
    return timer;
}
/* initializes search data structure */
searchdata_t *init_search_data(board_t *board) {
    searchdata_t *data = (searchdata_t *)malloc(sizeof(searchdata_t));

    data->board = copy_board(board);            /* pointer to the actual board */
    data->tt = init_tt(MB_TO_BYTES(64));        /* transposition table for the search */

    data->timer = init_timer();                 /* timer for time management */

    data->ponder = 0;                           /* tells engine to start search at ponder move */

    data->max_seldepth = -1;                    /* maximum depth searched while in quiescence search */
    data->best_move = NULL;                     /* best move in (iterative) search so far */
    data->best_eval = NEGINF;                   /* corresponding evaluation of best move */
    data->nodes_searched = 0;                   /* amount of nodes searched */
    data->hash_used = 0;                        /* amount of hash entries that lead to not */
                                                /* needing to search the node again */
    data->hash_bounds_adjusted = 0;             /* amount of hash entries that lead to */
                                                /* adjustment of alpha/beta bounds */
    data->pv_node_hit = 0;                      /* amount of pv moves that turned out to be the best move */
    return data;
}

/* frees search data structure */
void free_search_data(searchdata_t *data) {
    free(data->board);
    free_tt(data->tt);
    free_move(data->best_move);
    free(data);
}

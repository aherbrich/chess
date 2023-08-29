#include <stdio.h>
#include <sys/time.h>

#include "include/engine-core/search.h"

#include "include/engine-core/types.h"
#include "include/engine-core/move.h"
#include "include/engine-core/zobrist.h"
#include "include/engine-core/tt.h"
#include "include/engine-core/eval.h"
#include "include/engine-core/prettyprint.h"

#define NULL_MOVE_REDUCTION 2
#define PIECE_VALUE(X) (X == B_PAWN || X == W_PAWN) ? PAWNVALUE : (X == B_KNIGHT || X == W_KNIGHT) ? KNIGHTVALUE : (X == B_BISHOP || X == W_BISHOP) ? BISHOPVALUE : (X == B_ROOK || X == W_ROOK) ? ROOKVALUE : (X == B_QUEEN || X == W_QUEEN) ? QUEENVALUE : 20000

/* checks if the game is in late game, i.e. only kings and pawns are left */
int is_lategame(board_t *board) {
    for (int i = 0; i < 64; i++) {
        switch(board->playingfield[i]){
            case NO_PIECE:
            case W_KING:
            case B_KING:
            case W_PAWN:
            case B_PAWN:
                break;
            default:
                return 0;
        }
    }
    return 1;
}

/* creates score string for info output (for GUI) */
char *get_mate_or_cp_value(int score, int depth) {
    char *buffer = (char *)malloc(1024);
    for (int i = 0; i < 1024; i++) buffer[i] = '\0';

    if (score >= INF - MAXDEPTH) {
        snprintf(buffer, 8, "mate %d", (depth / 2));
    } else if (score <= NEGINF + MAXDEPTH) {
        snprintf(buffer, 8, "mate %d", -(depth / 2));
    } else {
        snprintf(buffer, 6, "cp %d", score);
    }
    return buffer;
}

/* determines a draw by threefold repitiion */
int draw_by_repition(board_t *board) {
    uint64_t current_board_hash = board->hash;

    int counter = 0;
    for (int i = 0; i < board->ply_no; i++) {
        if (board->history[i].hash == current_board_hash) counter++;
        if (counter == 2) {
            return 1;
        }
    }
    return 0;
}




/* quescience search */
int quiet_search(board_t *board, int alpha, int beta,
                 searchdata_t *searchdata, int depth, int ply) {
    searchdata->nodes_searched++;
    searchdata->max_seldepth = (searchdata->max_seldepth < depth) ? depth : searchdata->max_seldepth;
    /* check if we have exceeded the maximum search depth */
    if (ply >= MAXDEPTH) {
        return eval_board(searchdata->board);
    }

    /* check if we have exceeded the maximum nodes to search */
    if (searchdata->nodes_searched >= searchdata->timer.max_nodes) {
        searchdata->timer.stop = 1;
    }

    /* every so often, check if our time has expired */
    if ((searchdata->nodes_searched & STOP_ACCURACY) == 0) {
        check_time(&searchdata->timer);
    }

    /* If we have to stop, exit search by returning 0 in all branches.
       We will simply use the information of last search as our result
       and discard any information gained in this search. */
    if (searchdata->timer.stop == 1) {
        return 0;
    }

    int eval = eval_board(board);

    if (eval >= beta) {
        return eval;
    }
    if (eval > alpha) {
        alpha = eval;
    }

    int best_eval = eval;

    /* generate legal moves */
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);
    move_t move;

    while (!is_empty(&movelst)) {
        move = pop_max(&movelst);

        /* filter out non-captures */
        if (!(move.flags & 0b0100)) {
            continue;
        }

        // delta pruning
        // if (best_eval - 200 - is_capture(move->to, board) > eval) {
        //     free_move(move);
        //     continue;
        // }

        do_move(board, move);
        eval = -quiet_search(board, -beta, -alpha, searchdata, depth + 1, ply + 1);
        undo_move(board, move);

        /* if eval is better than the best so far, update it */
        if (eval > best_eval) {
            best_eval = eval;
        }
        /* if eval is better than alpha, adjust bound */
        if (eval > alpha) {
            alpha = eval;
        }
        /* beta-cutoff */
        if (eval >= beta) {
            break;
        }
    }

    return best_eval;
}

int negamax(searchdata_t *searchdata, int depth, int ply, int alpha, int beta, int allow_null_move) {
    searchdata->nodes_searched++;

    /* check if we have exceeded the maximum search depth */
    if (ply >= MAXDEPTH) {
        return eval_board(searchdata->board);
    }

    /* check if we have exceeded the maximum nodes to search */
    if (searchdata->nodes_searched >= searchdata->timer.max_nodes) {
        searchdata->timer.stop = 1;
    }

    /* every so often, check if our time has expired */
    if ((searchdata->nodes_searched & STOP_ACCURACY) == 0) {
        check_time(&searchdata->timer);
    }

    /* If we have to stop, exit search by returning 0 in all branches.
       We will simply use the information of last search as our result
       and discard any information gained in this search. */
    if (searchdata->timer.stop == 1) {
        return 0;
    }

    /* Check for draw by repitition or fifty move rule */
    if ((searchdata->board->history[searchdata->board->ply_no].fifty_move_counter >= 100 &&
         !(is_in_check(searchdata->board))) ||
        draw_by_repition(searchdata->board)) {
        return 0;
    }

    /* ================================================================== */
    /* CHECK EXTENSIONS: If the player is in check, we extend the search  */
    /* depth by one.                                                      */
    /* ================================================================== */
    if (is_in_check(searchdata->board)) {
        depth++;
    }

    /* ================================================================== */
    /* QUIESCENCE SEARCH: If the search depth is zero, we perform a       */
    /* quiescence search. This is a special search that only searches     */
    /* positions that are "quiet". A position is quiet if there are no    */
    /* immediate captures available. The idea is that we want to search   */
    /* deeper in positions that are not quiet, since they are more likely */
    /* to contain tactical lines and to prevent the horizion effect.      */
    /* ================================================================== */
    if (depth == 0) {
        return quiet_search(searchdata->board, alpha, beta, searchdata, 0, ply);
    }

    /* ================================================================== */
    /* TRANSPOSITION TABLE LOOKUP: We check if the current position is    */
    /* already stored in the transposition table. If so, we can use the   */
    /* information stored in the table to determine if we can prune the   */
    /* search tree.                                                       */
    /* ================================================================== */
    tt_entry_t* entry = retrieve_tt_entry(searchdata->tt, searchdata->board);

    if(entry && entry->depth >= depth) {
        int32_t pv_value = entry->eval;
        switch(entry->flags){
            /* ================================================================== */
            /* EXACT SCORE: If the score of the entry is exact, we can simply     */
            /* return the score.                                                  */
            /* ================================================================== */
            case EXACT:
                searchdata->pv_node_hit++;
                return pv_value;
            /* ================================================================== */
            /* LOWERBOUND: If the score of the entry is a lowerbound, i.e. has    */
            /* caused a beta cutoff before, we can return the score if it is      */
            /* greater than beta (since we would FAIL-HIGH ) a.k.a. trigger a     */
            /* beta cutoff again. BUT, we can not return the score if it is       */
            /* lower than beta, since the true value COULD be higher than beta    */
            /* (and fail-high), but also lower than beta (or even lower than      */
            /* alpha and fail-low) or in bewteen alpha and beta (in which case    */
            /* we would get an exact value)                                       */
            /* ================================================================== */
            case LOWERBOUND:
                searchdata->hash_bounds_adjusted++;
                if(pv_value >= beta) return pv_value;
                break;
            /* ================================================================== */
            /* UPPERBOUND: If the score of the entry is a upperbound, i.e. could  */
            /* not exceed alpha before because every child lead to a beta cutoff  */
            /* we can return the score if it is less than alpha (since we would   */
            /* FAIL-LOW) a.k.a. can't exceed alpha again. BUT, we can not return  */
            /* the score if it is greater than alpha, since the true value COULD  */
            /* be lower than alpha, but also higher than alpha (or even beta and  */
            /* fail high) or in between alpha and beta (in which case we would    */
            /* get an exact value)                                                */
            /* ================================================================== */
            case UPPERBOUND:
                searchdata->hash_bounds_adjusted++;
                if(pv_value <= alpha) return pv_value;
                break;
        }
    }

    /* ================================================================== */
    /* STACTIC NULL MOVE PRUNING: Similar to the null move heuristic, we  */
    /* try to reduce the search space by trying a "null" move, then see-  */
    /* ing if the score of the subtree is still high enough to cause a    */
    /* beta cutoff. BUT, instead of actually doing a search we simply     */
    /* subtract a large margin from the score (assume the opponent has a  */
    /* good move) and if the score is still greater than beta we assume   */
    /* the node will cause a beta cutoff and we can prune the branch. To  */
    /* reduce the likelihood of pruning an actually good move, the higher */
    /* we are in the search tree, the larger the margin we subtract.      */
    /* If we are in check we shouldnt use this technique, since (in       */
    /* theory) not making a move in check would result in an illegal      */
    /* position.                                                          */
    /* ================================================================== */
    if (!is_in_check(searchdata->board)){
        int32_t score = eval_board(searchdata->board);
        /* roughly one pawn margin for every ply */
        int32_t score_margin = 88 * depth;
        if (score - score_margin >= beta) {
            return score - score_margin;
        }
    }

    /* ================================================================== */
    /* NULL MOVE PRUNING: The Null Move Heuristic (NMH), is a method      */
    /* based on the null move observation to reduce the search space by   */
    /* trying a "null" move (letting the opponent play twice), then see-  */
    /* ing if the score of the subtree search is still high enough to     */
    /* cause a beta cutoff. The null move observation is the fact that in */
    /* chess, in almost every position, the side to move will have a move */ 
    /* to play that is better than doing nothing.                         */
    /* If we are in check we should not make null moves, since this       */
    /* would result in an illegal position. Likewise, two null moves in a */
    /* row are forbidden, since they result in nothing. Moreover, in late */
    /* game postions we should not make null moves either, because it is  */
    /* way more likely the null move observation does not hold due to     */
    /* zugzwang. Therefore we dont make null moves if there are only      */
    /* pawns and kings remaining on the board.                            */
    /* ================================================================== */
    if (allow_null_move && !is_in_check(searchdata->board) && depth >= 3 && !is_lategame(searchdata->board)){
        do_null_move(searchdata->board);
        int32_t score = -negamax(searchdata, depth - 1 - NULL_MOVE_REDUCTION, ply + 1, -beta, -beta + 1, 0);
        undo_null_move(searchdata->board);
        if (score >= beta) {
            return score;
        }
    }

    /* ================================================================== */
    /* MOVE ITERATION: A value is associated with each position of the    */
    /* game. This value is computed by means of an evaluation function    */
    /* and it indicates how good it would be for a player to reach that   */
    /* position. The player then iterates though all moves and chooses    */
    /* the move that maximizes the minimum value of the position          */
    /* resulting from the opponent's possible following moves.            */
    /* ================================================================== */
    maxpq_t movelst;
    move_t move;
    initialize_maxpq(&movelst);
    generate_moves(searchdata->board, &movelst);


    /* ================================================================== */
    /* PV-MOVE: We want to order the moves in a way that makes the        */
    /* search more efficient. We do this by searching the most promising  */
    /* moves first. the most important move ordering technique is to try  */
    /* PV-Moves first. A PV-Move is part of the principal variation and   */
    /* therefor a best move found in the previous iteration of an         */
    /* iterative deepening framework.                                     */
    /* ================================================================== */        
    if (entry) {
        for (int i = 1; i <= movelst.nr_elem; i++) {
            if (is_same_move(movelst.array[i], entry->best_move)) {
                movelst.array[i].value = 10000;
                swap(&movelst, i, 1);
                break;
            }
        }
    }

    int legal_moves = 0;
    int tt_flag = UPPERBOUND;
    int best_eval = NEGINF;
    move_t best_move = {0,0,0,0};

    while (!is_empty(&movelst)) {
        move = pop_max(&movelst);
        legal_moves++;

        do_move(searchdata->board, move);
        int eval = -negamax(searchdata, depth - 1, ply + 1, -beta, -best_eval, 1);
        undo_move(searchdata->board, move);

        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
        }

        /* alpha bound adjustment */
        if (eval > alpha) {
            alpha = eval;
            tt_flag = EXACT;
        }

        /* beta cutoff */
        if (alpha >= beta) {
            if (!is_empty(&movelst)) {
                tt_flag = LOWERBOUND;
            }
            
            break;
        }
    }

    /* If the player had no legal moves, the game is over (atleast in this branch of the search) */
    if (legal_moves == 0) {
        /* We wan't to determine if the player was check mated */
        if (is_in_check(searchdata->board)) {
            return NEGINF + depth;
        }
        /* Or if we reached a stalemate */
        else {
            return 0;
        }
    }

    /* ================================================================ */
    /* TRANSPOSITION TABLE STORING: We store the best move (and the     */
    /* search depth, it's evaluation and search flag) in the table.     */
    /* This information can then be used if we encounter the same       */
    /* postion again, and we don't need to search that branch any       */
    /* further. We should be careful, to only store this information    */
    /* if the search was not aborted by callee or due to time           */
    /* expiration (since we cant be sure that the information is truely */
    /* correct).                                                        */
    /* ================================================================ */
    if (searchdata->timer.stop != 1) {
        store_tt_entry(searchdata->tt, searchdata->board, best_move, depth, best_eval, tt_flag);
    }

    return best_eval;
}

void search(searchdata_t *searchdata) {
    /* Reset the history hash table from prevoius searches */
    /* Of course we keep the board hashes of already played positions untouched */
    for (int i = searchdata->board->ply_no; i < MAXPLIES; i++) {
        searchdata->board->history[i].hash = 0ULL;
    }

    /* Reset the performance counters and calculate the time available for search */
    searchdata->best_eval = NEGINF;
    searchdata->nodes_searched = 0;
    searchdata->hash_used = 0;
    searchdata->hash_bounds_adjusted = 0;
    searchdata->pv_node_hit = 0;
    searchdata->timer.time_available = calculate_time(searchdata);

    int alpha = NEGINF, beta = INF;

    /* =================================================================== */
    /* ITERATIVE DEEPINING: It has been noticed, that even if one is about */
    /* to search to a given depth, that iterative deepening is faster than */
    /* searching for the given depth immediately. This is due to dynamic   */
    /* move ordering techniques such as; PV- and hash- moves determined in */
    /* previous iteration(s), as well the history heuristic (TODO).        */
    /* =================================================================== */
    for (int depth = 1; depth <= searchdata->timer.max_depth && depth < MAXDEPTH;
         depth++) {
        int eval = negamax(searchdata, depth, 0, alpha, beta, 1);

        if (searchdata->timer.stop == 1) {
            if (searchdata->best_move == NULL && depth == 1) {
                searchdata->best_move = tt_best_move(searchdata->tt, searchdata->board);
            }
            break;
        }

        /* ================================================================ */
        /* ASPIRATION WINDOWS: The technique is to use a guess of the       */
        /* expected value (from the last iteration in iterative deepening)  */
        /* and use a window around this as the alpha-beta bounds. Because   */
        /* the window is narrower, more beta cutoffs are achieved, and the  */
        /* search takes a shorter time. The drawback is that if the true    */
        /* score is outside this window, then a costly re-search must be    */
        /* made. Typical window sizes are 1/2 to 1/4 of a pawn on either    */
        /* side of the guess.                                               */
        /* ================================================================ */
        if (eval <= alpha || eval >= beta) {
            alpha = NEGINF;
            beta = INF;
            depth--;
            continue;
        }

        alpha = eval - WINDOWSIZE;
        beta = eval + WINDOWSIZE;

        /* Update search data and output info (for GUI) */
        if(searchdata->best_move) free(searchdata->best_move);
        searchdata->best_move = tt_best_move(searchdata->tt, searchdata->board);
        searchdata->best_eval = eval;

        int nodes = searchdata->nodes_searched;
        int seldepth = searchdata->max_seldepth;
        int delta = delta_in_ms(searchdata);
        if(delta == 0) delta = 1;
        int nps = (int)(nodes / delta);
        int time = delta;
        int hashfull = tt_permille_full(searchdata->tt);
        char *score = get_mate_or_cp_value(eval, depth);

        printf("info score %s depth %d seldepth %d nodes %d time %d nps %d hasfull %d pv ",
               score, depth, seldepth, nodes, time, nps, hashfull);
        print_line(searchdata->tt, searchdata->board, depth);
        printf("\n");
        if (eval >= INF - MAXDEPTH || eval <= NEGINF + MAXDEPTH) break;

        free(score);
    }

    int nodes = searchdata->nodes_searched;
    int delta = delta_in_ms(searchdata);
    if(delta == 0) delta = 1;
    int nps = (int)(nodes / delta);
    int time = delta;
    int hashfull = tt_permille_full(searchdata->tt);
    char *move_str =
        get_LAN_move(*searchdata->best_move, searchdata->board->player);
    printf("info nodes %d time %d nps %d hasfull %d\nbestmove %s\n", nodes,
           time, nps, hashfull, move_str);
    printf("\n");
    free(move_str);
}

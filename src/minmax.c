#include "../include/chess.h"

int quietSearch(board_t *board, int alpha, int beta, clock_t start, double timeleft) {
    // evaluate board
    int value = evalBoardMax(board);

    // beta cutoff
    if (value >= beta) {
        return value;
    }

    node_t *movelst = generateCaptures(board);
    movelst = sortMoves(movelst);

    // if no more captures possible return evaluation
    if (len(movelst) == 0) {
        // else return evaluation
        return value;
    }

    move_t *move;
    player_t playeratturn = board->player;
    int bestvalue = value;

    // alpha beta only on captures
    while ((move = pop(movelst)) != NULL) {
        nodes_searched++;

        double timediff = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (timediff > timeleft) {
            free_move(move);
            free_movelst(movelst);
            uint64_t hash = zobrist(board);
            uint64_t key = hash % HTSIZE;
            return httable[key].eval;
        }

        alpha = maxof(value, alpha);

        playMove(board, move, playeratturn);
        int value = -quietSearch(board, -beta, -alpha, start, timeleft);
        reverseMove(board, move, playeratturn);

        if (value > bestvalue) {
            bestvalue = value;
            if (bestvalue >= beta) {
                free_move(move);
                goto betacutoff;
            }
        }
        free_move(move);
    }
betacutoff:

    // free the remaining move list
    while ((move = pop(movelst)) != NULL) {
        free_move(move);
    }
    free(movelst);

    return bestvalue;
}

int alphaBetaWithTT(board_t *board, uint8_t depth, int alpha, int beta, clock_t start, double timeleft) {
    int oldalpha = alpha;
    move_t *bestmovesofar = NULL;

    move_t *pvmove;
    int16_t pvvalue;
    int8_t pvflags;
    int8_t pvdepth;

    probeTableEntry(board, &pvflags, &pvvalue, &pvmove, &pvdepth);

    if (pvdepth == depth) {
        nodes_searched++;
        if (pvflags == FLG_EXCACT) {
            hash_used++;
            free_move(pvmove);
            return pvvalue;
        } else if (pvflags == FLG_CUT) {
            hash_boundsadjusted++;
            alpha = maxof(pvvalue, alpha);
        } else if (pvflags == FLG_ALL) {
            hash_boundsadjusted++;
            beta = minof(pvvalue, beta);
        }
        if (alpha >= beta) {
            hash_used++;
            free_move(pvmove);
            return pvvalue;
        }
    }

    if (depth == 0) {
        nodes_searched++;
        free_move(pvmove);
        return (quietSearch(board, alpha, beta, start, timeleft));
        // return evalBoardMax(board);
    }

    move_t *move;
    player_t playeratturn = board->player;
    int bestvalue;

    node_t *movelst = generateMoves(board);
    movelst = sortMoves(movelst);

    if (pvdepth >= 0 && PVMoveIsPossible(movelst, pvmove)) {
        nodes_searched++;

        playMove(board, pvmove, playeratturn);
        bestvalue = -alphaBetaWithTT(board, depth - 1, -beta, -alpha, start, timeleft);
        reverseMove(board, pvmove, playeratturn);

        free_move(bestmovesofar);
        bestmovesofar = copy_move(pvmove);
        if (bestvalue >= beta) {
            goto betacutoff;
        }
    } else {
        bestvalue = NEGINFINITY;
    }

    if (len(movelst) == 0) {
        nodes_searched++;
        free_move(pvmove);
        free_move(bestmovesofar);
        free_movelst(movelst);
        return evalEndOfGameMax(board, depth);
    }

    while ((move = pop(movelst)) != NULL) {
        nodes_searched++;

        double timediff = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (timediff > timeleft) {
            free_move(move);
            free_movelst(movelst);
            return pvvalue;
        }

        if (pvmove != NULL && isSameMove(move, pvmove)) {
            free_move(move);
            continue;
        }

        alpha = maxof(bestvalue, alpha);
        playMove(board, move, playeratturn);
        int value = -alphaBetaWithTT(board, depth - 1, -beta, -alpha, start, timeleft);
        reverseMove(board, move, playeratturn);

        if (value > bestvalue) {
            bestvalue = value;
            free_move(bestmovesofar);
            bestmovesofar = copy_move(move);

            if (bestvalue >= beta) {
                free_move(move);
                goto betacutoff;
            }
        }
        free_move(move);
    }

betacutoff:

    if (bestvalue <= oldalpha) {
        storeTableEntry(board, FLG_ALL, bestvalue, bestmovesofar, depth);
    } else if (bestvalue >= beta) {
        storeTableEntry(board, FLG_CUT, bestvalue, bestmovesofar, depth);
    } else {
        storeTableEntry(board, FLG_EXCACT, bestvalue, bestmovesofar, depth);
    }

    /* free the remaining move list */
    free_movelst(movelst);

    /* free ttmove & bestmovesofar */
    free_move(pvmove);
    free_move(bestmovesofar);

    return bestvalue;
}

move_t *iterativeSearch(board_t *board, int8_t maxdepth, double maxtime) {
    /* reset the performance counters */
    nodes_searched = 0;
    hash_used = 0;
    hash_boundsadjusted = 0;

    double timeleft = maxtime;
    move_t *bestmove = NULL;
    clock_t begin;

    for (int i = 1; i <= maxdepth; i++) {
        begin = clock();
        alphaBetaWithTT(board, i, NEGINFINITY, INFINITY, clock(), timeleft);
        uint64_t hash = zobrist(board);
        hash = hash % HTSIZE;
        free_move(bestmove);
        bestmove = copy_move(httable[hash].bestmove);
        clock_t end = clock();

        // printMove(bestmove);
        //  printf("\t\t\t Time:\t%fs\n",
        //          (double)(end - begin) / CLOCKS_PER_SEC);

        // printLine(board, i);
        // printf("\n");

        timeleft -= (double)(end - begin) / CLOCKS_PER_SEC;
        if (timeleft <= 0) {
            printf("Depth searched: %d\n", i);
            break;
        }
    }

    return bestmove;
}
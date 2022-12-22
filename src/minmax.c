 #include "../include/chess.h"

int alphaBetaWithTT(board_t *board, uint8_t depth, int alpha, int beta){
    int oldalpha = alpha;
    move_t* bestmovesofar = NULL;

    uint64_t hash = zobrist(board);
    uint64_t hashmod = hash%HTSIZE;

    move_t* ttmove = copy_move(httable[hashmod].bestmove);
    int ttvalue = httable[hashmod].eval;
    int8_t ttflags = httable[hashmod].flags;
    int8_t ttdepth = httable[hashmod].depth;

    if(ttdepth == depth){
        nodes_searched++;
        if(ttflags == FLG_EXCACT){
            hash_used++;
            free_move(ttmove);
            return ttvalue;
        }
        else if(ttflags == FLG_CUT){
            hash_boundsadjusted++;
            alpha = maxof(ttvalue, alpha);
        }
        else if(ttflags == FLG_ALL){
            hash_boundsadjusted++;
            beta = minof(ttvalue, beta);
        }
        if (alpha >= beta){
            hash_used++;
            free_move(ttmove);
            return ttvalue;
        }
    }

    if(depth == 0){
        nodes_searched++;
        free_move(ttmove);
        return evalBoardMax(board);
    }
    
    move_t *move;
    player_t playeratturn = board->player;
    int bestvalue;

    node_t* movelst = generateMoves(board);
    movelst = sortMoves(movelst);

    if(ttdepth >= 0 && ttMoveIsPossible(movelst, ttmove)){

        nodes_searched++;
        playMove(board, ttmove, playeratturn);
        bestvalue = -alphaBetaWithTT(board, depth-1, -beta, -alpha);
        reverseMove(board, ttmove, playeratturn);

        free_move(bestmovesofar);
        bestmovesofar = copy_move(ttmove);
        if(bestvalue >= beta){
            goto betacutoff;
        }
    }
    else{
        bestvalue = NEGINFINITY;
    }

    //bestvalue = NEGINFINITY;

    if(len(movelst) == 0){
        nodes_searched++;
        free_move(ttmove);
        free_move(bestmovesofar);
        free(movelst);
        return evalEndOfGameMax(board, depth);
    }

    while((move = pop(movelst)) != NULL){
        nodes_searched++;

        if(ttmove != NULL && isSameMove(move, ttmove)){
            free_move(move);
            continue;
        }

        alpha = maxof(bestvalue, alpha);
        playMove(board, move, playeratturn);
        int value = -alphaBetaWithTT(board, depth-1, -beta, -alpha);
        reverseMove(board, move, playeratturn);

        if(value > bestvalue){
            bestvalue = value;
            free_move(bestmovesofar);
            bestmovesofar = copy_move(move);
            
            if(bestvalue >= beta){
                free_move(move);
                goto betacutoff;
            }
        }
        free_move(move);
    }

    betacutoff:

    httable[hashmod].depth = depth;
    httable[hashmod].eval = bestvalue;
    httable[hashmod].hash = hash;
    free_move(httable[hashmod].bestmove); // freeing ttmove
    httable[hashmod].bestmove = copy_move(bestmovesofar);

    if(bestvalue <= oldalpha){
        httable[hashmod].flags = FLG_ALL;
    }
    else if(bestvalue >= beta){
        httable[hashmod].flags = FLG_CUT;
    }
    else{
        httable[hashmod].flags = FLG_EXCACT;
    }

    /* free the remaining move list */
    while((move = pop(movelst)) != NULL) {
        free_move(move);
    }
    free(movelst);
    /* free ttmove & bestmovesofar */
    free_move(ttmove);
    free_move(bestmovesofar);

    return bestvalue;
}
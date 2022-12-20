 #include "../include/chess.h"

int alphaBeta(board_t *board, int depth, int alpha, int beta, int maximizingplayer, int maxdepth, move_t** bestmove){
    // generate possible moves
    node_t *movelst = generateMoves(board);
    movelst = sort_byorder(movelst);

    // if player cant move
    if(len(movelst) == 0){
        // check for stalemate
        board->player = OPPONENT(board->player);
        int inCheck = !isLegalMove(board);
        board->player = OPPONENT(board->player);

        // can check
        if(inCheck){
            // if WHITE (the maximizing player) is in check
            // return negative evaluation
            if(board->player == WHITE){
                return -20000-depth;
            } 
            // if BLACK (the minimizing player) is in check
            // return positiv evaluation
            else{
                return 20000+depth;
            }
        }
        // stalemate has been reached
        else{
            return 0;
        }
    }
    // or max depth has been reached
    if (depth == 0 ){
        return evalBoard(board);
    }


    if(maximizingplayer){
        int value = NEGINFINITY;

        move_t *move;
        player_t playerwhomademove= board->player;

        while((move= pop(movelst)) != NULL){
            playMove(board, move, playerwhomademove);
            uint64_t hash = zobrist(&zobtable, board);
            int eval = alphaBeta(board, depth-1, alpha, beta, FALSE, maxdepth, bestmove);
            reverseMove(board, move, playerwhomademove);

            // value = max(value, eval)
            if (eval > value) {
                value = eval;
                if(depth == maxdepth){
                    //printf("Hash: %llu\n", hash);
                    *bestmove = move;
                }
            }

            // beta cutoff
            if (value >= beta){
                break;
            }

            // alpha = max(value, alpha)
            if (value > alpha){
                alpha = value;
            }
        }

        return value;
    }
    else{
        int value = INFINITY;

        move_t *move;
        player_t playerwhomademove= board->player;

        while((move= pop(movelst)) != NULL){
            playMove(board, move, playerwhomademove);
            uint64_t hash = zobrist(&zobtable, board);
            int eval = alphaBeta(board, depth-1, alpha, beta, TRUE, maxdepth, bestmove);
            reverseMove(board, move, playerwhomademove);


            // value = min(value, eval)
            if (eval < value) {
                value = eval;
                if(depth == maxdepth){
                    //printf("Hash: %llu\n", hash);
                    *bestmove = move;
                }
            }

            // alpha cutoff
            if (value <= alpha){
                break;
            }

            // beta = min(value, alpha)
            if (value < beta){
                beta = value;
            }
        }

        return value;
    }
    
}
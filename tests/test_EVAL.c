#include "../include/chess.h"

char STARTING_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char TEST2_FEN[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
char TEST22_FEN[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - ";
char TEST3_FEN[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
char TEST4_FEN[] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
char TEST5_FEN[] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
char TEST6_FEN[] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
char TEST7_FEN[] = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";
char TEST8_FEN[] = "1k6/4Q3/2Q5/8/8/8/6K1/8 w - - 0 1";


////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT

int main(){
    board_t* board = init_board();
    loadByFEN(board, TEST4_FEN);

    clock_t end;
    clock_t begin;
    clock_t tmpbegin;

    printBoard(board);



    begin = clock();
    tmpbegin = clock();

    /* Implement here */
    init_zobrist(&zobtable);


    move_t *bestmove = NULL;
    int maxdepth = 6;
    
    int evaluation;

    for(int i = 0; i < 10; i++){
        printf("Player to find best move: %d\n", board->player);
        evaluation = alphaBeta(board, maxdepth, NEGINFINITY, INFINITY, MAXIMIZING(board->player), maxdepth, &bestmove);

        printf("Found (eval:%d):\n", evaluation);
        printMove(bestmove);

        end = clock();
        printf("\t\t\t Time:\t%fs\n", (double)(end - tmpbegin) / CLOCKS_PER_SEC);
        tmpbegin = clock();
        printf("\n");

        playMove(board, bestmove, board->player);
        printBoard(board);
    }




    ///////////
    end = clock();
    printf("Overall Time: \t\t%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);   
    
    free_board(board);
}
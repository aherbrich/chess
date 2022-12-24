#include <time.h>

#include "../include/chess.h"
#include "../include/prettyprint.h"

#define Color_YELLOW "\033[0;33m"
#define Color_GREEN "\033[0;32m"
#define Color_RED "\033[0;31m"
#define Color_CYAN "\033[0;36m"
#define Color_PURPLE "\033[0;35m"
#define Color_WHITE "\033[0;37m"

#define Color_END "\033[0m"

char *STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char TEST2_FEN[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
char TEST3_FEN[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
char TEST4_FEN[] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
char TEST5_FEN[] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
char TEST6_FEN[] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
char TEST7_FEN[] = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";

int main() {
    int result[] = {20, 400, 8902, 197281, 4865609, 119060324};
    board_t* board = init_board();
    loadByFEN(board, STARTING_FEN);

    clock_t end;
    clock_t begin;

    int genresult;

    printBoard(board);

    printf("TESTING START POSITION...\n");

    int fail_counter = 0;

    for (unsigned long i = 0; i < (sizeof(result) / sizeof(int)); i++) {
        printf("%s", Color_END);
        begin = clock();
        printf("DEPTH(%d):\n", (int)i + 1);
        printf("Expected: \t%d\n", result[i]);
        genresult = MoveGen(board, i + 1);
        printf("Found: \t\t%d\n", genresult);
        end = clock();
        printf("Time: \t\t%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);

        if (result[i] == genresult) {
            printf("%sOk...%s\n\n", Color_GREEN, Color_END);
        } else {
            fail_counter++;
            printf("%sFailed...%s\n\n", Color_RED, Color_END);
        }
    }
    printf("\n");
    free_board(board);
    if (fail_counter != 0) {
        exit(1);
    }
}
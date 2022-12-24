#include <string.h>

#include "../include/chess.h"

char *STARTING_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char TEST2_FEN[] =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
char TEST22_FEN[] =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - ";
char TEST3_FEN[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
char TEST4_FEN[] =
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
char TEST5_FEN[] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
char TEST6_FEN[] =
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
char TEST7_FEN[] = "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1";
char TEST8_FEN[] = "1k6/4Q3/2Q5/8/8/8/6K1/8 w - - 0 1";
char TEST9_FEN[] = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -";

int nodes_searched = 0;
int hash_used = 0;
int hash_boundsadjusted = 0;

////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT

idx_t inputToIdx(char *ptr) {
    piece_t idx = 0;
    while (*ptr != '\0') {
        switch (*ptr) {
            case 'a':
                idx = idx + 0;
                break;
            case 'b':
                idx = idx + 1;
                break;
            case 'c':
                idx = idx + 2;
                break;
            case 'd':
                idx = idx + 3;
                break;
            case 'e':
                idx = idx + 4;
                break;
            case 'f':
                idx = idx + 5;
                break;
            case 'g':
                idx = idx + 6;
                break;
            case 'h':
                idx = idx + 7;
                break;
            case 'A':
                idx = idx + 0;
                break;
            case 'B':
                idx = idx + 1;
                break;
            case 'C':
                idx = idx + 2;
                break;
            case 'D':
                idx = idx + 3;
                break;
            case 'E':
                idx = idx + 4;
                break;
            case 'F':
                idx = idx + 5;
                break;
            case 'G':
                idx = idx + 6;
                break;
            case 'H':
                idx = idx + 7;
                break;
            case '1':
                idx = idx + (8 * 0);
                break;
            case '2':
                idx = idx + (8 * 1);
                break;
            case '3':
                idx = idx + (8 * 2);
                break;
            case '4':
                idx = idx + (8 * 3);
                break;
            case '5':
                idx = idx + (8 * 4);
                break;
            case '6':
                idx = idx + (8 * 5);
                break;
            case '7':
                idx = idx + (8 * 6);
                break;
            case '8':
                idx = idx + (8 * 7);
                break;
        }
        ptr++;
    }
    return (idx);
}

move_t *getMove(board_t *board) {
    char inputbuffer[20];
    printf("Enter move: ");
    scanf("%s", inputbuffer);

    char *fromstr = strtok(inputbuffer, "-");
    char *tostr = strtok(NULL, " ");
    idx_t from = inputToIdx(fromstr);
    idx_t to = inputToIdx(tostr);

    node_t *movelst = generateMoves(board);

    node_t *tmp = movelst->next;
    move_t *move = NULL;

    while (tmp != NULL) {
        if (tmp->move->start == from && tmp->move->end == to) {
            move = tmp->move;
            break;
        }
        tmp = tmp->next;
    }

    if (move) {
        return move;
    } else {
        printf("Invalid move!\n");
        return getMove(board);
    }
}

int main() {
    board_t *board = init_board();
    loadByFEN(board, STARTING_FEN);

    clock_t end;
    clock_t begin;

    /* Implement here */
    init_zobrist();
    init_hashtable();
    init_book();

    printBoard(board);

    while (len(generateMoves(board)) > 0) {
        if (board->player == WHITE) {
            move_t *move = getMove(board);
            playMove(board, move, board->player);
            board->plynr++;
            printBoard(board);
        } else {
            printf("\n%sBOT at play\n\n", Color_PURPLE);

            nodes_searched = 0;
            hash_used = 0;
            hash_boundsadjusted = 0;

            int maxdepth = 40;
            double maxtime = 5.0;

            begin = clock();

            clear_hashtable();
            move_t *bestmove;

            if (book_possible(board) == 1) {
                printf("Book move possible!\n");
                bestmove = getRandomBook(board);
            } else {
                printf("No book moves possible!\n");
                bestmove = iterativeSearch(board, maxdepth, maxtime);

                printf("\nNodes Explored:\t%d\n", nodes_searched);
                printf("Hashes used:\t%d \t(%4.2f)\n", hash_used, (float)hash_used / (float)nodes_searched);
                printf("Bounds adj.:\t%d\n", hash_boundsadjusted);
                printf("Found move:\t");
                printMove(bestmove);

                end = clock();
                printf("\t\t\t Time:\t%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);
                printf("\n");
            }

            playMove(board, bestmove, board->player);
            board->plynr++;
            printBoard(board);
            printf("%s", Color_END);
        }
    }

    free_board(board);
}
#include <string.h>

#include "../include/book.h"
#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"

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

////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT

move_t *getMove(board_t *board) {
    char input_buffer[20];
    printf("Enter move: ");
    scanf("%s", input_buffer);
    move_t *move = str_to_move(board, input_buffer);

    if (move) {
        return move;
    } else {
        printf("Invalid move!\n");
        return getMove(board);
    }
}

int main() {
    board_t *board = init_board();
    load_by_FEN(board, STARTING_FEN);

    clock_t end;
    clock_t begin;

    /* Implement here */
    init_zobrist();
    init_hashtable();
    init_book();

    print_board(board);

    while (len(generate_moves(board)) > 0) {
        if (board->player == WHITE) {
            move_t *move = getMove(board);
            play_move(board, move, board->player);
            board->ply_no++;
            print_board(board);
        } else {
            printf("\n%sBOT at play\n\n", Color_PURPLE);

            nodes_searched = 0;
            hash_used = 0;
            hash_bounds_adjusted = 0;

            search_data* data = init_search_data(board);
            data->max_depth = 40;
            data->max_time = 5000;

            begin = clock();

            clear_hashtable();
            move_t *best_move;

            if (book_possible(board) == 1) {
                printf("Book move possible!\n");
                best_move = get_random_book(board);
            } else {
                printf("No book moves possible!\n");
                /* wont work in future but needed files to compile */
                best_move = iterative_search(data);

                printf("\nNodes Explored:\t%d\n", nodes_searched);
                printf("Hashes used:\t%d \t(%4.2f)\n", hash_used, (float)hash_used / (float)nodes_searched);
                printf("Bounds adj.:\t%d\n", hash_bounds_adjusted);
                printf("Found move:\t");
                print_move(best_move);

                end = clock();
                printf("\t\t\t Time:\t%fs\n", (double)(end - begin) / CLOCKS_PER_SEC);
                printf("\n");
            }

            play_move(board, best_move, board->player);
            board->ply_no++;
            print_board(board);
            printf("%s", Color_END);
        }
    }

    free_board(board);
}
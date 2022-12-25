#include "../include/chess.h"
#include "../include/zobrist.h"

char FIELD[64][2] = {"A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
                     "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
                     "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
                     "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
                     "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
                     "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
                     "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
                     "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"};

/////////////////////////////////////////////////////////////
//  PRINT HELPERS

/* Print a piece */
char printPiece(piece_t piece) {
    switch (piece) {
        case 2:
            return 'P';
        case 4:
            return 'N';
        case 8:
            return 'B';
        case 16:
            return 'R';
        case 32:
            return 'Q';
        case 64:
            return 'K';
        case 3:
            return 'p';
        case 5:
            return 'n';
        case 9:
            return 'b';
        case 17:
            return 'r';
        case 33:
            return 'q';
        case 65:
            return 'k';
        default:
            return '-';
    }
}

/* Print the board */
void printBoard(board_t* board) {
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                printf("%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }

            char piece = printPiece(board->playing_field[posToIdx(x, y)]);
            if (COLOR(board->playing_field[posToIdx(x, y)]) == BLACK) {
                printf("%s", Color_CYAN);
            }
            printf("%c%s  ", piece, Color_END);
        }
        printf("\n");
    }
    printf("\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN, Color_END);
    printf("\n");
}

/* Print move*/
void printMove(move_t* move) {
    char* startfield = FIELD[move->start];
    char* endfield = FIELD[move->end];
    if (move->typeofmove == PROMOTIONMOVE) {
        printf("%.2s-%.2s-(%c)", startfield, endfield, printPiece(move->piece_is));

    } else if (move->typeofmove == CASTLEMOVE) {
        printf("%.2s-%.2s-C", startfield, endfield);
    } else {
        printf("%.2s-%.2s-%c", startfield, endfield, printPiece(move->piece_was));
    }
}

/* Print list move */
void printMoves(node_t* movelst) {
    node_t* ptr = movelst->next;
    while (ptr != NULL) {
        printMove(ptr->move);
        printf("\n");
        ptr = ptr->next;
    }
    printf("\n");
}

void printLine(board_t* board, int depth) {
    uint64_t hash;
    uint64_t hashmod;
    player_t playeratturn = board->player;
    move_t* bestmove;

    if (depth > 0) {
        hash = zobrist(board);
        hashmod = hash % HTSIZE;

        bestmove = copy_move(httable[hashmod].bestmove);

        if (bestmove == NULL) {
            return;
        }
        playMove(board, bestmove, playeratturn);
        printMove(bestmove);
        printf("\t");
        printLine(board, depth - 1);
        reverseMove(board, bestmove, playeratturn);
        free_move(bestmove);
    } else {
        return;
    }
}
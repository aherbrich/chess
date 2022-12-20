#include "../include/chess.h"
#include <string.h>

char STARTING_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT

idx_t inputToIdx(char *ptr){
    piece_t idx = 0;
    while(*ptr != '\0'){
        switch(*ptr){
            case 'a': idx = idx + 0; break;
            case 'b': idx = idx + 1; break;
            case 'c': idx = idx + 2; break;
            case 'd': idx = idx + 3; break;
            case 'e': idx = idx + 4; break;
            case 'f': idx = idx + 5; break;
            case 'g': idx = idx + 6; break;
            case 'h': idx = idx + 7; break;
            case 'A': idx = idx + 0; break;
            case 'B': idx = idx + 1; break;
            case 'C': idx = idx + 2; break;
            case 'D': idx = idx + 3; break;
            case 'E': idx = idx + 4; break;
            case 'F': idx = idx + 5; break;
            case 'G': idx = idx + 6; break;
            case 'H': idx = idx + 7; break;
            case '1': idx = idx + (8 * 0); break;
            case '2': idx = idx + (8 * 1); break;
            case '3': idx = idx + (8 * 2); break;
            case '4': idx = idx + (8 * 3); break;
            case '5': idx = idx + (8 * 4); break;
            case '6': idx = idx + (8 * 5); break;
            case '7': idx = idx + (8 * 6); break;
            case '8': idx = idx + (8 * 7); break;
        }
        ptr++;
    }
    return(idx);
}


move_t* getMove(board_t *board){
    char inputbuffer[20];
    printf("Enter move: ");
    scanf("%s", inputbuffer);

    char *fromstr = strtok(inputbuffer, "-");
    char *tostr = strtok(NULL, " ");
    idx_t from = inputToIdx(fromstr);
    idx_t to = inputToIdx(tostr);

    node_t* movelst = generateMoves(board);

    node_t* tmp = movelst->next;
    move_t* move = NULL;

    while(tmp != NULL){
        if(tmp->move->start == from && tmp->move->end == to){
            move = tmp->move;
            break;
        }
        tmp = tmp->next;
    }

    if(move){
        return move;
    }
    else{
        printf("Invalid move!\n");
        return getMove(board);
    }

}
int main(){
    board_t* board = init_board();
    loadByFEN(board, STARTING_FEN);

    clock_t end;
    clock_t begin;
    clock_t tmpbegin;

    printBoard(board);

    while(len(generateMoves(board)) > 0){
        if(board->player == WHITE){
            move_t *move = getMove(board);
            playMove(board, move, board->player);
            printBoard(board);
        }
        else{
            printf("\n%sBOT at play\n\n", Color_PURPLE);
            move_t *bestmove = NULL;
            int maxdepth = 5;
    
            int evaluation;
            evaluation = alphaBeta(board, maxdepth, NEGINFINITY, INFINITY, MAXIMIZING(board->player), maxdepth, &bestmove);
            playMove(board, bestmove, board->player);
            printBoard(board);
            printf("%s", Color_END);
        }
    }
    
        
    free_board(board);
}
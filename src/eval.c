#include "../include/chess.h"

#define PAWNVALUE 100
#define KNIGHTVALUE 320
#define BISHOPVALUE 330
#define ROOKVALUE 500
#define QUEENVALUE 900

// pawn position values
int pawnpos[64] = { 0,  0,  0,  0,  0,  0,  0,  0,
                    50, 50, 50, 50, 50, 50, 50, 50,
                    10, 10, 20, 30, 30, 20, 10, 10,
                    5,  5, 10, 25, 25, 10,  5,  5,
                    0,  0,  0, 20, 20,  0,  0,  0,
                    5, -5,-10,  0,  0,-10, -5,  5,
                    5, 10, 10,-20,-20, 10, 10,  5,
                    0,  0,  0,  0,  0,  0,  0,  0};

// knight position values
int knightpos[64] = {   -50,-40,-30,-30,-30,-30,-40,-50,
                        -40,-20,  0,  0,  0,  0,-20,-40,
                        -30,  0, 10, 15, 15, 10,  0,-30,
                        -30,  5, 15, 20, 20, 15,  5,-30,
                        -30,  0, 15, 20, 20, 15,  0,-30,
                        -30,  5, 10, 15, 15, 10,  5,-30,
                        -40,-20,  0,  5,  5,  0,-20,-40,
                        -50,-40,-30,-30,-30,-30,-40,-50};

// bishop position values
int bishoppos[64] = {-20,-10,-10,-10,-10,-10,-10,-20,
                        -10,  0,  0,  0,  0,  0,  0,-10,
                        -10,  0,  5, 10, 10,  5,  0,-10,
                        -10,  5,  5, 10, 10,  5,  5,-10,
                        -10,  0, 10, 10, 10, 10,  0,-10,
                        -10, 10, 10, 10, 10, 10, 10,-10,
                        -10,  5,  0,  0,  0,  0,  5,-10,
                        -20,-10,-10,-10,-10,-10,-10,-20};

// rook position values
int rookpos[64] = {  0,  0,  0,  0,  0,  0,  0,  0,
                        5, 10, 10, 10, 10, 10, 10,  5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        0,  0,  0,  5,  5,  0,  0,  0};

// queen position values
int queenpos[64] = {  0,  0,  0,  0,  0,  0,  0,  0,
                        5, 10, 10, 10, 10, 10, 10,  5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        -5,  0,  0,  0,  0,  0,  0, -5,
                        0,  0,  0,  5,  5,  0,  0,  0};

// king position values
int kingpos[64] = {  -30,-40,-40,-50,-50,-40,-40,-30,
                        -30,-40,-40,-50,-50,-40,-40,-30,
                        -30,-40,-40,-50,-50,-40,-40,-30,
                        -30,-40,-40,-50,-50,-40,-40,-30,
                        -20,-30,-30,-40,-40,-30,-30,-20,
                        -10,-20,-20,-20,-20,-20,-20,-10,
                        20, 20,  0,  0,  0,  0, 20, 20,
                        20, 30, 10,  0,  0, 10, 30, 20};

int countMaterial(board_t *board, player_t color){
    int material = 0;
    
    for(int x = 0; x<8; x++){
        for(int y = 0; y < 8; y++){
            piece_t piece = board->playingfield[posToIdx(x, y)];
            if(piece == (PAWN|color)){
                material += PAWNVALUE;
                if(color == WHITE){
                    material += pawnpos[posToIdx(7-x, y)];
                }
                else{
                    material += pawnpos[63-posToIdx(7-x, y)];
                }
            }
            else if(piece == (KNIGHT|color)){
                material += KNIGHTVALUE;
                if(color == WHITE){
                    material += knightpos[posToIdx(7-x, y)];
                }
                else{
                    material += knightpos[63-posToIdx(7-x, y)];
                }
            }
            else if(piece == (BISHOP|color)){
                material += BISHOPVALUE;
                if(color == WHITE){
                    material += bishoppos[posToIdx(7-x, y)];
                }
                else{
                    material += bishoppos[63-posToIdx(7-x, y)];
                }
            }
            else if(piece == (ROOK|color)){
                material += ROOKVALUE;
                if(color == WHITE){
                    material += rookpos[posToIdx(7-x, y)];
                }
                else{
                    material += rookpos[63-posToIdx(7-x, y)];
                }
            }
            else if(piece == (QUEEN|color)){
                material += QUEENVALUE;
                if(color == WHITE){
                    material += queenpos[posToIdx(7-x, y)];
                }
                else{
                    material += queenpos[63-posToIdx(7-x, y)];
                }
            }
            else if(piece == (KING|color)){
                if(color == WHITE){
                    material += kingpos[posToIdx(7-x, y)];
                }
                else{
                    material += kingpos[63-posToIdx(7-x, y)];
                }
            }
        }
    }

    return material;
}


int evalBoard(board_t* board){
    int whiteEval = countMaterial(board, WHITE);
    int blackEval = countMaterial(board, BLACK);

    int eval = whiteEval - blackEval;

    // if(board->player == WHITE){
    //     eval = eval * -1;
    // }
    return eval;
}


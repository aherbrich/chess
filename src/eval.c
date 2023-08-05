#include "../include/eval.h"

#include "../include/chess.h"

/* pawn position values */
int PAWN_POSITION_VALUE[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

/* knight position values */
int KNIGHT_POSITION_VALUE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};

/* bishop position values */
int BISHOP_POSITION_VALUE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};

/* rook position values */
int ROOK_POSITION_VALUE[64] = {0,  0,  0, 0,  0, 0,  0,  0, 5,  10, 10, 10, 10,
                               10, 10, 5, -5, 0, 0,  0,  0, 0,  0,  -5, -5, 0,
                               0,  0,  0, 0,  0, -5, -5, 0, 0,  0,  0,  0,  0,
                               -5, -5, 0, 0,  0, 0,  0,  0, -5, -5, 0,  0,  0,
                               0,  0,  0, -5, 0, 0,  0,  5, 5,  0,  0,  0};

/* queen position values */
int QUEEN_POSITION_VALUE[64] = {0,  0,  0, 0,  0, 0,  0,  0, 5,  10, 10, 10, 10,
                                10, 10, 5, -5, 0, 0,  0,  0, 0,  0,  -5, -5, 0,
                                0,  0,  0, 0,  0, -5, -5, 0, 0,  0,  0,  0,  0,
                                -5, -5, 0, 0,  0, 0,  0,  0, -5, -5, 0,  0,  0,
                                0,  0,  0, -5, 0, 0,  0,  5, 5,  0,  0,  0};

/* king position values */
int KING_POSITION_VALUE[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

const int MATERIAL_VALUE[16] = {-PAWNVALUE, -KNIGHTVALUE, -BISHOPVALUE, -ROOKVALUE, -QUEENVALUE, 0, 0, 0,
                            PAWNVALUE, KNIGHTVALUE, BISHOPVALUE, ROOKVALUE, QUEENVALUE, 0, 0, 0};


const double FEATURE_WEIGHTS[210] = {
0.846,
0.683,
0.673,
0.835,
0.814,
0.813,
0.997,
1.023,
1.108,
1.095,
1.118,
1.050,
1.034,
1.040,
0.955,
0.930,
0.917,
0.916,
0.839,
0.858,
0.748,
0.866,
0.670,
0.819,
0.566,
0.923,
0.372,
0.620,
0.309,
0.000,
0.125,
0.173,
0.078,
0.117,
0.087,
0.103,
0.136,
0.107,
0.104,
0.088,
0.091,
0.096,
0.104,
0.061,
0.071,
0.069,
0.075,
0.079,
0.089,
0.076,
0.092,
0.066,
0.087,
0.052,
0.069,
0.035,
0.049,
0.038,
0.012,
0.010,
0.154,
0.081,
-0.030,
0.026,
0.066,
0.061,
0.083,
0.120,
0.092,
0.097,
0.087,
0.065,
0.126,
0.105,
0.145,
0.118,
0.129,
0.121,
0.103,
0.129,
0.108,
0.117,
0.082,
0.084,
0.095,
0.096,
0.093,
0.082,
0.086,
0.083,
0.404,
0.026,
-0.129,
-0.106,
-0.006,
0.013,
0.039,
0.086,
0.045,
0.101,
0.084,
0.141,
0.240,
0.147,
0.204,
0.202,
0.175,
0.172,
0.154,
0.172,
0.134,
0.139,
0.111,
0.130,
0.023,
0.079,
-0.016,
0.055,
-0.008,
0.114,
0.241,
-0.051,
-0.075,
0.107,
0.073,
0.075,
0.048,
0.055,
0.169,
0.173,
0.166,
0.312,
0.214,
0.328,
0.285,
0.377,
0.352,
0.400,
0.223,
0.306,
0.205,
0.208,
0.136,
0.110,
0.024,
0.140,
0.408,
0.194,
0.154,
0.442,
-0.680,
-0.351,
-0.037,
-0.163,
-0.497,
0.019,
-0.125,
0.148,
-0.009,
0.066,
0.165,
0.130,
0.173,
0.102,
-0.039,
0.153,
0.025,
-0.092,
-0.091,
0.025,
-0.078,
-0.224,
-0.232,
-0.199,
-0.277,
-0.363,
-0.140,
-0.274,
-0.046,
-0.002,
-0.066,
-0.001,
-0.018,
-0.038,
-0.060,
-0.067,
-0.018,
-0.075,
-0.085,
-0.075,
-0.070,
-0.027,
-0.036,
-0.017,
0.004,
0.045,
0.063,
0.062,
0.095,
0.087,
0.114,
0.083,
0.082,
0.086,
0.104,
0.085,
0.101,
0.134,
0.143,
0.144,
};

/* Number of pieces */
int nr_of_pieces(board_t *board) {
    int counter = 0;

    for(int i = 0; i < 64; i++){
        if(board->playingfield[i] != NO_PIECE) counter++;
    }

    return counter;
}

/* Evaluates a board (when game is over) */
/* WARNING: Use only if game is over i.e. draw/check mate*/
int eval_end_of_game(board_t *board, int depth) {
    /* check for stalemate */
    int in_check = is_in_check(board);

    /* if check (mate) delivered */
    if (in_check) {
        return -16000 - depth;
    }
    /* if stalemate */
    else {
        return 0;
    }
}

/* Evaluates a board (when game is not over) */
/* WARNING: Use only if game is not over i.e. no draw/no checkmate*/
// int eval_board(board_t *board) {
//     int idx = nr_of_pieces(board)-3;
//     double material = 0.0;
//     double positional = 0.0;

//     for(int i = 0; i < 64; i++){
//         material += MATERIAL_VALUE[board->playingfield[i]] * FEATURE_WEIGHTS[idx];
//         switch(board->playingfield[i]){
//             case B_PAWN:
//                 positional -= PAWN_POSITION_VALUE[i] * FEATURE_WEIGHTS[30+idx];
//                 break;
//             case B_KNIGHT:
//                 positional -= KNIGHT_POSITION_VALUE[i] * FEATURE_WEIGHTS[60+idx];
//                 break;
//             case B_BISHOP:
//                 positional -= BISHOP_POSITION_VALUE[i] * FEATURE_WEIGHTS[90+idx];
//                 break;  
//             case B_ROOK:
//                 positional -= ROOK_POSITION_VALUE[i] * FEATURE_WEIGHTS[120+idx];
//                 break;
//             case B_QUEEN:
//                 positional -= QUEEN_POSITION_VALUE[i] * FEATURE_WEIGHTS[150+idx];
//                 break;
//             case B_KING:
//                 positional -= KING_POSITION_VALUE[i] * FEATURE_WEIGHTS[180+idx];
//                 break;
//             case W_PAWN:
//                 positional += PAWN_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[30+idx];
//                 break;
//             case W_KNIGHT:
//                 positional += KNIGHT_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[60+idx];
//                 break;
//             case W_BISHOP:
//                 positional += BISHOP_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[90+idx];
//                 break;
//             case W_ROOK:
//                 positional += ROOK_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[120+idx];
//                 break;
//             case W_QUEEN:
//                 positional += QUEEN_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[150+idx];
//                 break;
//             case W_KING:
//                 positional += KING_POSITION_VALUE[63 - i] * FEATURE_WEIGHTS[180+idx];
//                 break;
//             default:
//                 break;
//         }
//     }

//     int eval = (int) (material + positional); 

//     /* due to negamax we want both players to maximize */
//     if (board->player == BLACK) {
//         eval *= -1;
//     }

//     return eval;
// }

int eval_board(board_t *board) {
    int material = 0;
    int positional = 0;

    for(int i = 0; i < 64; i++){
        material += MATERIAL_VALUE[board->playingfield[i]];
        switch(board->playingfield[i]){
            case B_PAWN:
                positional -= PAWN_POSITION_VALUE[i];
                break;
            case B_KNIGHT:
                positional -= KNIGHT_POSITION_VALUE[i];
                break;
            case B_BISHOP:
                positional -= BISHOP_POSITION_VALUE[i];
                break;  
            case B_ROOK:
                positional -= ROOK_POSITION_VALUE[i];
                break;
            case B_QUEEN:
                positional -= QUEEN_POSITION_VALUE[i];
                break;
            case B_KING:
                positional -= KING_POSITION_VALUE[i];
                break;
            case W_PAWN:
                positional += PAWN_POSITION_VALUE[63 - i];
                break;
            case W_KNIGHT:
                positional += KNIGHT_POSITION_VALUE[63 - i];
                break;
            case W_BISHOP:
                positional += BISHOP_POSITION_VALUE[63 - i];
                break;
            case W_ROOK:
                positional += ROOK_POSITION_VALUE[63 - i];
                break;
            case W_QUEEN:
                positional += QUEEN_POSITION_VALUE[63 - i];
                break;
            case W_KING:
                positional += KING_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }

    int eval = material + positional;

    /* due to negamax we want both players to maximize */
    if (board->player == BLACK) {
        eval *= -1;
    }

    return eval;
}
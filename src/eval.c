#include "../include/chess.h"

#define PAWNVALUE 100
#define KNIGHTVALUE 320
#define BISHOPVALUE 330
#define ROOKVALUE 500
#define QUEENVALUE 900

/* pawn position values */
int PAWN_POSITION_VALUE[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                   50, 50, 50, 50, 50, 50, 50, 50,
                   10, 10, 20, 30, 30, 20, 10, 10,
                   5, 5, 10, 25, 25, 10, 5, 5,
                   0, 0, 0, 20, 20, 0, 0, 0,
                   5, -5, -10, 0, 0, -10, -5, 5,
                   5, 10, 10, -20, -20, 10, 10, 5,
                   0, 0, 0, 0, 0, 0, 0, 0};

/* knight position values */
int KNIGHT_POSITION_VALUE[64] = {-50, -40, -30, -30, -30, -30, -40, -50,
                     -40, -20, 0, 0, 0, 0, -20, -40,
                     -30, 0, 10, 15, 15, 10, 0, -30,
                     -30, 5, 15, 20, 20, 15, 5, -30,
                     -30, 0, 15, 20, 20, 15, 0, -30,
                     -30, 5, 10, 15, 15, 10, 5, -30,
                     -40, -20, 0, 5, 5, 0, -20, -40,
                     -50, -40, -30, -30, -30, -30, -40, -50};

/* bishop position values */
int BISHOP_POSITION_VALUE[64] = {-20, -10, -10, -10, -10, -10, -10, -20,
                     -10, 0, 0, 0, 0, 0, 0, -10,
                     -10, 0, 5, 10, 10, 5, 0, -10,
                     -10, 5, 5, 10, 10, 5, 5, -10,
                     -10, 0, 10, 10, 10, 10, 0, -10,
                     -10, 10, 10, 10, 10, 10, 10, -10,
                     -10, 5, 0, 0, 0, 0, 5, -10,
                     -20, -10, -10, -10, -10, -10, -10, -20};

/* rook position values */
int ROOK_POSITION_VALUE[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                   5, 10, 10, 10, 10, 10, 10, 5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   0, 0, 0, 5, 5, 0, 0, 0};

/* queen position values */
int QUEEN_POSITION_VALUE[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                    5, 10, 10, 10, 10, 10, 10, 5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    0, 0, 0, 5, 5, 0, 0, 0};

/* king position values */
int KING_POSITION_VALUE[64] = {-30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -20, -30, -30, -40, -40, -30, -30, -20,
                   -10, -20, -20, -20, -20, -20, -20, -10,
                   20, 20, 0, 0, 0, 0, 20, 20,
                   20, 30, 10, 0, 0, 10, 30, 20};

/* Sums up the material and positional values of each piece */
int simple_eval(board_t *board, player_t color) {
    int material = 0;
    bitboard_t pawns, knights, bishops, rooks, queens, king;

    if(color == WHITE){
        pawns = board->whitepawns;
        knights = board->whiteknights;
        bishops = board->whitebishops;
        rooks = board->whiterooks;
        queens = board->whitequeens;
        king = board->whiteking;
    } else{
        pawns = board->blackpawns;
        knights = board->blackknights;
        bishops = board->blackbishops;
        rooks = board->blackrooks;
        queens = board->blackqueens;
        king = board->blackking;
    }

    while(pawns){
        material += PAWNVALUE;
        material += PAWN_POSITION_VALUE[pop_1st_bit(&pawns)];
    }
    while(knights){
        material += KNIGHTVALUE;
        material += KNIGHT_POSITION_VALUE[pop_1st_bit(&knights)];
    }
    while(bishops){
        material += BISHOPVALUE;
        material += BISHOP_POSITION_VALUE[pop_1st_bit(&bishops)];
    }
    while(rooks){
        material += ROOKVALUE;
        material += ROOK_POSITION_VALUE[pop_1st_bit(&rooks)];
    }
    while(queens){
        material += QUEENVALUE;
        material += KNIGHT_POSITION_VALUE[pop_1st_bit(&queens)];
    }
    while(king){
        if(color == WHITE) {
            material += KING_POSITION_VALUE[pop_1st_bit(&king)];
        } else{
            material += KING_POSITION_VALUE[63 - pop_1st_bit(&king)];
        }
    }

    return material;
}

/* Evaluates game over situations */
int eval_end_of_game(board_t *board, int depth) {
    /* check for stalemate */
    int in_check = is_in_check(board);

    /* if in check */
    if (in_check) {
        return -16000 - depth;
    }
    /* stalemate has been reached */
    else {
        return 0;
    }
}

/* Evaluates a board (used when game is not over) */
int eval_board(board_t *board) {
    int white_eval = simple_eval(board, WHITE);
    int black_eval = simple_eval(board, BLACK);

    int eval = white_eval - black_eval;

    if (board->player == BLACK) {
        eval *= -1;
    }

    return eval;
}
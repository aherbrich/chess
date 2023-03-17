#include "../include/chess.h"

#define PAWNVALUE 100
#define KNIGHTVALUE 320
#define BISHOPVALUE 330
#define ROOKVALUE 500
#define QUEENVALUE 900

// pawn position values
int pawnpos[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                   50, 50, 50, 50, 50, 50, 50, 50,
                   10, 10, 20, 30, 30, 20, 10, 10,
                   5, 5, 10, 25, 25, 10, 5, 5,
                   0, 0, 0, 20, 20, 0, 0, 0,
                   5, -5, -10, 0, 0, -10, -5, 5,
                   5, 10, 10, -20, -20, 10, 10, 5,
                   0, 0, 0, 0, 0, 0, 0, 0};

// knight position values
int knightpos[64] = {-50, -40, -30, -30, -30, -30, -40, -50,
                     -40, -20, 0, 0, 0, 0, -20, -40,
                     -30, 0, 10, 15, 15, 10, 0, -30,
                     -30, 5, 15, 20, 20, 15, 5, -30,
                     -30, 0, 15, 20, 20, 15, 0, -30,
                     -30, 5, 10, 15, 15, 10, 5, -30,
                     -40, -20, 0, 5, 5, 0, -20, -40,
                     -50, -40, -30, -30, -30, -30, -40, -50};

// bishop position values
int bishoppos[64] = {-20, -10, -10, -10, -10, -10, -10, -20,
                     -10, 0, 0, 0, 0, 0, 0, -10,
                     -10, 0, 5, 10, 10, 5, 0, -10,
                     -10, 5, 5, 10, 10, 5, 5, -10,
                     -10, 0, 10, 10, 10, 10, 0, -10,
                     -10, 10, 10, 10, 10, 10, 10, -10,
                     -10, 5, 0, 0, 0, 0, 5, -10,
                     -20, -10, -10, -10, -10, -10, -10, -20};

// rook position values
int rookpos[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                   5, 10, 10, 10, 10, 10, 10, 5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   -5, 0, 0, 0, 0, 0, 0, -5,
                   0, 0, 0, 5, 5, 0, 0, 0};

// queen position values
int queenpos[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                    5, 10, 10, 10, 10, 10, 10, 5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    -5, 0, 0, 0, 0, 0, 0, -5,
                    0, 0, 0, 5, 5, 0, 0, 0};

// king position values
int kingpos[64] = {-30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -30, -40, -40, -50, -50, -40, -40, -30,
                   -20, -30, -30, -40, -40, -30, -30, -20,
                   -10, -20, -20, -20, -20, -20, -20, -10,
                   20, 20, 0, 0, 0, 0, 20, 20,
                   20, 30, 10, 0, 0, 10, 30, 20};

int count_material(board_t *board, player_t color) {
    int material = 0;

    /* determine the value of all pieces of a specific color on the board */
    /* each piece has a (1) static value 
                        (2) dynamic positional value
    */
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            piece_t piece = board->playing_field[pos_to_idx(x, y)];
            if (piece == (PAWN | color)) {
                material += PAWNVALUE;
                if (color == WHITE) {
                    material += pawnpos[pos_to_idx(7 - x, y)];
                } else {
                    material += pawnpos[63 - pos_to_idx(7 - x, y)];
                }
            } else if (piece == (KNIGHT | color)) {
                material += KNIGHTVALUE;
                if (color == WHITE) {
                    material += knightpos[pos_to_idx(7 - x, y)];
                } else {
                    material += knightpos[63 - pos_to_idx(7 - x, y)];
                }
            } else if (piece == (BISHOP | color)) {
                material += BISHOPVALUE;
                if (color == WHITE) {
                    material += bishoppos[pos_to_idx(7 - x, y)];
                } else {
                    material += bishoppos[63 - pos_to_idx(7 - x, y)];
                }
            } else if (piece == (ROOK | color)) {
                material += ROOKVALUE;
                if (color == WHITE) {
                    material += rookpos[pos_to_idx(7 - x, y)];
                } else {
                    material += rookpos[63 - pos_to_idx(7 - x, y)];
                }
            } else if (piece == (QUEEN | color)) {
                material += QUEENVALUE;
                if (color == WHITE) {
                    material += queenpos[pos_to_idx(7 - x, y)];
                } else {
                    material += queenpos[63 - pos_to_idx(7 - x, y)];
                }
            } else if (piece == (KING | color)) {
                if (color == WHITE) {
                    material += kingpos[pos_to_idx(7 - x, y)];
                } else {
                    material += kingpos[63 - pos_to_idx(7 - x, y)];
                }
            }
        }
    }

    return material;
}

int eval_end_of_game(board_t *board, int depth) {
    // check for stalemate
    board->player = OPPONENT(board->player);  // same as doing a null move
    int inCheck = !is_legal_move(board);      // are we in check?
    board->player = OPPONENT(board->player);  // reverse the null move

    // if in check return the worst possible evaluation
    if (inCheck) {
        return -16000 - depth;
    }
    // else stalemate has been reached and evaluate to a draw
    else {
        return 0;
    }
}

int eval_board(board_t *board) {
    int whiteEval = count_material(board, WHITE);
    int blackEval = count_material(board, BLACK);

    /* eval = NEGATIVE if black has advantage 
            = POSTIVE  if white has advantage */
    int eval = whiteEval - blackEval;

    /* since each player is maximizing */
    if (board->player == BLACK) {
        /* we might have to adjust the eval depending on who is playing */
        eval = eval * -1;
    }
    return eval;
}
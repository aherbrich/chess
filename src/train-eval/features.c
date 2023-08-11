#include "../../include/types.h"
#include "../../include/eval.h"
#include "../../include/linalg.h"

double material_difference(board_t* board){
    int material = 0;
    for(int i = 0; i < 64; i++){
        material += MATERIAL_VALUE[board->playingfield[i]];
    }
    return ((double) material) / 800.0;
}

double positional_difference(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
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
    return ((double) positional) / 300.0;
}

double positional_diff_pawn(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_PAWN:
                positional -= PAWN_POSITION_VALUE[i];
                break;
            case W_PAWN:
                positional += PAWN_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

double positional_diff_knight(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_KNIGHT:
                positional -= KNIGHT_POSITION_VALUE[i];
                break;
            case W_KNIGHT:
                positional += KNIGHT_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

double positional_diff_bishop(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_BISHOP:
                positional -= BISHOP_POSITION_VALUE[i];
                break;
            case W_BISHOP:
                positional += BISHOP_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

double positional_diff_rook(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_ROOK:
                positional -= ROOK_POSITION_VALUE[i];
                break;
            case W_ROOK:
                positional += ROOK_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

double positional_diff_queen(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_QUEEN:
                positional -= QUEEN_POSITION_VALUE[i];
                break;
            case W_QUEEN:
                positional += QUEEN_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

double positional_diff_king(board_t* board){
    int positional = 0;
    for(int i = 0; i < 64; i++){
        switch(board->playingfield[i]){
            case B_KING:
                positional -= KING_POSITION_VALUE[i];
                break;
            case W_KING:
                positional += KING_POSITION_VALUE[63 - i];
                break;
            default:
                break;
        }
    }
    return ((double) positional) / 30.0;
}

/* Calculates feature matrix */
void calculate_feautures(board_t *board, matrix_t *X, int idx) {
    int nr_pieces = nr_of_pieces(board);

    matrix_set(X, material_difference(board), idx, nr_pieces-3); // 2-3 = 0 smallest idx, 32-3 = 29 largest index
    matrix_set(X, positional_diff_pawn(board), idx, 30+(nr_pieces-3));
    matrix_set(X, positional_diff_knight(board), idx, 60+(nr_pieces-3));
    matrix_set(X, positional_diff_bishop(board), idx, 90+(nr_pieces-3));
    matrix_set(X, positional_diff_rook(board), idx, 120+(nr_pieces-3));
    matrix_set(X, positional_diff_queen(board), idx, 150+(nr_pieces-3));
    matrix_set(X, positional_diff_king(board), idx, 180+(nr_pieces-3));
}
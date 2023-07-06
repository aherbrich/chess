#include "../include/chess.h"
#include "../include/eval.h"
#include "../include/linalg.h"

int nr_of_pieces(board_t* board){
    int counter = 0;
    bitboard_t all = board->all;

    while(all){
        counter++;
        pop_1st_bit(&all);
    }

    return counter;
}

double pawn_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t pawns;
    if(color == WHITE){
        pawns = board->whitepawns;
    } else{
        pawns = board->blackpawns;
    }
    while(pawns){
        if(color == WHITE) {
            positional += PAWN_POSITION_VALUE[63-pop_1st_bit(&pawns)];
        } else{
            positional += PAWN_POSITION_VALUE[pop_1st_bit(&pawns)];
        }
    }

    return (double) positional;
}

double knight_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t knights;
    if(color == WHITE){
        knights = board->whiteknights;
    } else{
        knights = board->blackknights;
    }
    while(knights){
        if(color == WHITE) {
            positional += KNIGHT_POSITION_VALUE[63-pop_1st_bit(&knights)];
        } else{
            positional += KNIGHT_POSITION_VALUE[pop_1st_bit(&knights)];
        }
    }

    return (double) positional;
}

double bishop_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t bishops;
    if(color == WHITE){
        bishops = board->whitebishops;
    } else{
        bishops = board->blackbishops;
    }
    while(bishops){
        if(color == WHITE) {
            positional += BISHOP_POSITION_VALUE[63-pop_1st_bit(&bishops)];
        } else{
            positional += BISHOP_POSITION_VALUE[pop_1st_bit(&bishops)];
        }
    }

    return (double) positional;
}

double rook_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t rooks;
    if(color == WHITE){
        rooks = board->whiterooks;
    } else{
        rooks = board->blackrooks;
    }
    while(rooks){
        if(color == WHITE) {
            positional += ROOK_POSITION_VALUE[63-pop_1st_bit(&rooks)];
        } else{
            positional += ROOK_POSITION_VALUE[pop_1st_bit(&rooks)];
        }
    }

    return (double) positional;
}

double queen_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t queens;
    if(color == WHITE){
        queens = board->whitequeens;
    } else{
        queens = board->blackqueens;
    }
    while(queens){
        if(color == WHITE) {
            positional += QUEEN_POSITION_VALUE[63-pop_1st_bit(&queens)];
        } else{
            positional += QUEEN_POSITION_VALUE[pop_1st_bit(&queens)];
        }
    }

    return (double) positional;
}

double king_positional(board_t *board, player_t color){
    int positional = 0;
    bitboard_t king;
    if(color == WHITE){
        king = board->whiteking;
    } else{
        king = board->blackking;
    }
    while(king){
        if(color == WHITE) {
            positional += KING_POSITION_VALUE[63-pop_1st_bit(&king)];
        } else{
            positional += KING_POSITION_VALUE[pop_1st_bit(&king)];
        }
    }

    return (double) positional;
}

double all_positional(board_t *board, player_t color){
    double positional = 0.0;
    positional += pawn_positional(board, color);
    positional += knight_positional(board, color);
    positional += bishop_positional(board, color);
    positional += rook_positional(board, color);
    positional += queen_positional(board, color);
    positional += king_positional(board, color);
    return positional;
}

double positional_difference(board_t *board, player_t color){
    double positional_diff = all_positional(board, WHITE) - all_positional(board, BLACK);

    // if (board->player == BLACK) {
    //     return -positional_diff;
    // }
    return positional_diff/300.0;
}

double pawn_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t pawns;
    if(color == WHITE){
        pawns = board->whitepawns;
    } else{
        pawns = board->blackpawns;
    }
    while(pawns){
        material += PAWNVALUE;
        pop_1st_bit(&pawns);
    }

    return (double) material;
}

double knight_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t knights;
    if(color == WHITE){
        knights = board->whiteknights;
    } else{
        knights = board->blackknights;
    }
    while(knights){
        material += PAWNVALUE;
        pop_1st_bit(&knights);
    }

    return (double) material;
}

double bishop_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t bishops;
    if(color == WHITE){
        bishops = board->whitebishops;
    } else{
        bishops = board->blackbishops;
    }
    while(bishops){
        material += PAWNVALUE;
        pop_1st_bit(&bishops);
    }

    return (double) material;
}

double rook_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t rooks;
    if(color == WHITE){
        rooks = board->whiterooks;
    } else{
        rooks = board->blackrooks;
    }
    while(rooks){
        material += PAWNVALUE;
        pop_1st_bit(&rooks);
    }

    return (double) material;
}

double queen_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t queens;
    if(color == WHITE){
        queens = board->whitequeens;
    } else{
        queens = board->blackqueens;
    }
    while(queens){
        material += PAWNVALUE;
        pop_1st_bit(&queens);
    }

    return (double) material;
}

double king_material(board_t *board, player_t color){
    int material = 0;
    bitboard_t king;
    if(color == WHITE){
        king = board->whiteking;
    } else{
        king = board->blackking;
    }
    while(king){
        material += PAWNVALUE;
        pop_1st_bit(&king);
    }

    return (double) material;
}

double all_material(board_t *board, player_t color){
    double material = 0.0;
    material += pawn_material(board, color);
    material += knight_material(board, color);
    material += bishop_material(board, color);
    material += rook_material(board, color);
    material += queen_material(board, color);
    material += king_material(board, color);
    return material;
}


double material_difference(board_t *board, player_t color){
    double material_diff = all_material(board, WHITE) - all_material(board, BLACK);

    // if (board->player == BLACK) {
    //     return -material_diff;
    // }
    return material_diff/800.0;
}

double white_king_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whiteking) return 1.0;
    return 0.0;
}
double white_queen_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whitequeens) return 1.0;
    return 0.0;
} 
double white_rook_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whiterooks) return 1.0;
    return 0.0;
} 
double white_bishop_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whitebishops) return 1.0;
    return 0.0;
} 
double white_knight_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whiteknights) return 1.0;
    return 0.0;
} 
double white_pawn_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->whitepawns) return 1.0;
    return 0.0;
} 

double black_king_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackking) return -1.0;
    return 0.0;
}
double black_queen_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackqueens) return -1.0;
    return 0.0;
} 
double black_rook_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackrooks) return -1.0;
    return 0.0;
} 
double black_bishop_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackbishops) return -1.0;
    return 0.0;
} 
double black_knight_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackknights) return -1.0;
    return 0.0;
} 
double black_pawn_loc(board_t *board, idx_t idx){
    bitboard_t mask = 1ULL << idx;
    if(mask & board->blackpawns) return -1.0;
    return 0.0;
} 

double mobility(board_t *board){
     maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    double nr_moves = (double) (&movelst)->nr_elem;
    free_pq(&movelst);

    return nr_moves/50.0;
}

void calculate_feautures(board_t *board, matrix_t* X, int idx){
    int nr_pieces = nr_of_pieces(board);
    
    for(int j = 0; j < 30; j++){
        for(int i = 0; i < 64; i++){
            if((j+3) == nr_pieces){
                matrix_set(X, white_king_loc(board, i), idx, i+(64*j)+(1920*0));
                matrix_set(X, white_queen_loc(board, i), idx, i+(64*j)+(1920*1));
                matrix_set(X, white_rook_loc(board, i), idx, i+(64*j)+(1920*2));
                matrix_set(X, white_knight_loc(board, i), idx, i+(64*j)+(1920*3));
                matrix_set(X, white_bishop_loc(board, i), idx, i+(64*j)+(1920*4));
                matrix_set(X, white_pawn_loc(board, i), idx, i+(64*j)+(1920*5));

                matrix_set(X, black_king_loc(board, i), idx, i+(64*j)+(1920*6));
                matrix_set(X, black_queen_loc(board, i), idx, i+(64*j)+(1920*7));
                matrix_set(X, black_rook_loc(board, i), idx, i+(64*j)+(1920*8));
                matrix_set(X, black_knight_loc(board, i), idx, i+(64*j)+(1920*9));
                matrix_set(X, black_bishop_loc(board, i), idx, i+(64*j)+(1920*10));
                matrix_set(X, black_pawn_loc(board, i), idx, i+(64*j)+(1920*11));
            }
        }
    }
}
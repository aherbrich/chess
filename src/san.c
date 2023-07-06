#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/san.h"

/* Returns true if character is a file (i.e a-h) */
int is_file(char character){
    return (character >= 97 && character <= 104);
}

/* Returns true if character is a chess piece (i.e B,K,N,Q,R) */
int is_piece(char character){
    return (character == 66 || character == 75 || character == 78 || character == 81 || character == 82);
}

/* Converts a string to an index */
idx_t str_to_idx(char file, char rank) {
    idx_t idx = 0;
    idx = idx + ((file) - 'a');
    idx = idx + (8 * (rank - '1'));
    return idx;
}

/* Determines pawn move equal to the move described by flags */
move_t* find_pawn_move(board_t* board, char file1, char file2, char rank2, int capture, int promotion, char promopiece){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a pawn move 
        bitboard_t from_mask = 1ULL << move->from;
        if((from_mask & board->whitepawns) || (from_mask & board->blackpawns)){
            // check if move is FROM the correct file and TO the correct square
            if(move->to == to && (move->from%8+'a') == file1){
                // if non-promotion
                if(!promotion){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if promotion
                else{
                    if(promopiece == 'Q' && (move->flags == QPROM || move->flags == QCPROM)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    if(promopiece == 'R' && (move->flags == RPROM || move->flags == RCPROM)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    if(promopiece == 'B' && (move->flags == BPROM || move->flags == BCPROM)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    if(promopiece == 'N' && (move->flags == KPROM || move->flags == KCPROM)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        } 
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines castle move equal to the move described by flags */
move_t* find_castle_move(board_t* board, int kingside){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];
        // if kingside castle found
        if(kingside && move->flags == KCASTLE){
            move_t *copy = copy_move(move);
            free_pq(&movelst);
            return copy;
        } 
        // if queenside castle found
        else if(!kingside && move->flags == QCASTLE){
            move_t *copy = copy_move(move);
            free_pq(&movelst);
            return copy;
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines knight move equal to the move described by flags */
move_t* find_knight_move(board_t* board, char file1, char rank1, char file2, char rank2, int capture,  int single_ambigious, int double_ambigious){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);
    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a knight move 
        bitboard_t from_mask = 1ULL << move->from;
        if(((from_mask & board->whiteknights) || (from_mask & board->blackknights))){
            // check if TO square is the correct TO square
            if(move->to == to){
                // if move is unambigious, we can instantly return
                if(!single_ambigious && !double_ambigious){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if move is ambigious by rank and file
                else if(double_ambigious){
                    // check if file AND rank of FROM square is correct 
                    if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                } else if(single_ambigious){
                    // check if file OR rank of FROM square is correct 
                    if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines bishop move equal to the move described by flags */
move_t* find_bishop_move(board_t* board, char file1, char rank1, char file2, char rank2, int capture,  int single_ambigious, int double_ambigious){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a bishop move 
        bitboard_t from_mask = 1ULL << move->from;
        if(((from_mask & board->whitebishops) || (from_mask & board->blackbishops))){
            // check if TO square is the correct TO square
            if(move->to == to){
                // if move is unambigious, we can instantly return
                if(!single_ambigious && !double_ambigious){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if move is ambigious by rank and file
                else if(double_ambigious){
                    // check if file AND rank of FROM square is correct 
                    if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                } else if(single_ambigious){
                    // check if file OR rank of FROM square is correct 
                    if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines rook move equal to the move described by flags */
move_t* find_rook_move(board_t* board, char file1, char rank1, char file2, char rank2, int capture,  int single_ambigious, int double_ambigious){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a rook move 
        bitboard_t from_mask = 1ULL << move->from;
        if(((from_mask & board->whiterooks) || (from_mask & board->blackrooks))){
            // check if TO square is the correct TO square
            if(move->to == to){
                // if move is unambigious, we can instantly return
                if(!single_ambigious && !double_ambigious){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if move is ambigious by rank and file
                else if(double_ambigious){
                    // check if file AND rank of FROM square is correct 
                    if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                } else if(single_ambigious){
                    // check if file OR rank of FROM square is correct 
                    if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines queen move equal to the move described by flags */
move_t* find_queen_move(board_t* board, char file1, char rank1, char file2, char rank2, int capture, int single_ambigious, int double_ambigious){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a queen move 
        bitboard_t from_mask = 1ULL << move->from;
        if(((from_mask & board->whitequeens) || (from_mask & board->blackqueens))){
            // check if TO square is the correct TO square
            if(move->to == to){
                // if move is unambigious, we can instantly return
                if(!single_ambigious && !double_ambigious){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if move is ambigious by rank and file
                else if(double_ambigious){
                    // check if file AND rank of FROM square is correct 
                    if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                } else if(single_ambigious){
                    // check if file OR rank of FROM square is correct 
                    if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Determines king move equal to the move described by flags */
move_t* find_king_move(board_t* board, char file1, char rank1, char file2, char rank2, int capture, int single_ambigious, int double_ambigious){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);

    // determine index of to square
    idx_t to = str_to_idx(file2, rank2);

    // iterate through all moves 
    for(int i = 1; i < (&movelst)->nr_elem+1; i++){
        move_t* move = (&movelst)->array[i];

        // check if move is a king move 
        bitboard_t from_mask = 1ULL << move->from;
        if(((from_mask & board->whiteking) || (from_mask & board->blackking))){
            // check if TO square is the correct TO square
            if(move->to == to){
                // if move is unambigious, we can instantly return
                if(!single_ambigious && !double_ambigious){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                // if move is ambigious by rank and file
                else if(double_ambigious){
                    // check if file AND rank of FROM square is correct 
                    if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                } else if(single_ambigious){
                    // check if file OR rank of FROM square is correct 
                    if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            }
        }
    }
    free_pq(&movelst);
    return NULL;
}

/* Converts a string (short algebraic notation) to a move */
move_t* str_to_move(board_t* board, char* token){
    move_t* move = NULL;
    int idx = 0;

    // if pawn move
    if(is_file(token[idx])){
        char file1 = token[idx];
        char file2;
        char rank2;
        int capture = 0;
        int promotion = 0;
        char promopiece = '-';
        
        idx++;
        // if capture
        if(token[idx] == 'x'){
            capture = 1;
            idx++;
            file2 = token[idx];
            idx++;
            rank2 = token[idx];
        } 
        // if non-capture
        else{
            file2 = file1;
            rank2 = token[idx];
        }
        idx++;
        // if promotion
        if(token[idx] == '='){
            promotion = 1;
            idx++;
            promopiece = token[idx];
        }
        
        
        // FIND FITTING MOVE
        move = find_pawn_move(board, file1, file2, rank2, capture, promotion, promopiece);

    } 
    // if piece move
    else if (is_piece(token[idx])){
        char file1;
        char file2;
        char rank1;
        char rank2;
        char piece = token[idx];
        int capture = 0;
        int single_ambigious = 0; // -1 if by rank, 1 if by file
        int double_ambigious = 0;

        idx++;
        if(is_file(token[idx])){
            file1 = token[idx];
            idx++;
            if(is_file(token[idx])){
                single_ambigious = AMBIG_BY_FILE;
                file2 = token[idx];
                rank2 = token[idx+1];
            } else if(token[idx] == 'x'){
                capture = 1;
                single_ambigious = AMBIG_BY_FILE;
                file2 = token[idx+1];
                rank2 = token[idx+2];
            } else{
                rank1 = token[idx];
                if(is_file(token[idx+1])){
                    double_ambigious = 1;
                    file2 = token[idx+1];
                    rank2 = token[idx+2];
                } else if(token[idx +1] == 'x'){
                    capture = 1;
                    double_ambigious = 1;
                    file2 = token[idx+2];
                    rank2 = token[idx+3];
                } else{
                    file2 = file1;
                    rank2 = rank1;
                }
            }
        } else if (token[idx] == 'x'){
            capture = 1;
            file2 = token[idx+1];
            rank2 = token[idx+2];
        } else{
            single_ambigious = AMBIG_BY_RANK;
            rank1 = token[idx];
            if(token[idx+1] == 'x'){
                capture = 1;
                file2 = token[idx+2];
                rank2 = token[idx+3];
            } else{
                file2 = token[idx+1];
                rank2 = token[idx+2];
            }
        }


        // FIND FITTING MOVE
        if(piece == 'N'){
            move = find_knight_move(board, file1, rank1, file2, rank2, capture, single_ambigious, double_ambigious);
        }else if(piece == 'B'){
            move = find_bishop_move(board, file1, rank1, file2, rank2, capture,  single_ambigious, double_ambigious);
        }else if(piece == 'R'){
            move = find_rook_move(board, file1, rank1, file2, rank2, capture,  single_ambigious, double_ambigious);
        }else if(piece == 'Q'){
            move = find_queen_move(board, file1, rank1, file2, rank2, capture,  single_ambigious, double_ambigious);
        } else{
            move = find_king_move(board, file1, rank1, file2, rank2, capture,  single_ambigious, double_ambigious);
        }
    } 
    // if castle move
    else if (token[0] == 'O'){
        // assume kingside castle
        int kingside = 1;

        // if queenside castle though, change variable
        if(token[3]=='-'){
            kingside = 0;
        }


        // FIND FITTING MOVE
        move =  find_castle_move(board, kingside);
    }

    // check if we found a move
    if(!move){
        fprintf(stderr, "THIS SHOULD NOT HAPPEN! MOVE %s SHOULD BE POSSIBLE!\n", token);
    }
    
    return move;
}


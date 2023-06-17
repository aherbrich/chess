#include "../include/chess.h"
#include "../include/prettyprint.h"
#include "../include/zobrist.h"
#include <string.h>
#include <math.h>

#define WINWHITE 1
#define DRAW 0
#define WINBLACK -1
#define AMBIG_BY_RANK -1
#define AMBIG_BY_FILE 1

board_t* OLDSTATE[2048];
uint64_t HISTORY_HASHES[2048];

typedef struct _chessgame_t{
    char* movelist;
    int winner;
} chessgame_t;

int counter_number_of_games(){
    FILE *fp = fopen("/Users/aherbrich/src/myprojects/chess/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn", "r");
    int game_count = 0;
    int buffersize = 8192;
    char buffer[buffersize];

    while(fgets(buffer, buffersize, fp) != NULL){
        if(!strcmp(buffer, "\r\n")){
            fgets(buffer, buffersize, fp);
            if(strstr(buffer, "1.")){
                game_count++;
            }
            fgets(buffer, buffersize, fp);
        }
           
    }

    fclose(fp);

    return game_count;
}


chessgame_t** parse_chessgames_file(int nr_of_games){
    chessgame_t **chessgames = (chessgame_t**) malloc(sizeof(chessgame_t*) * nr_of_games);

    for(int i = 0; i < nr_of_games; i++){
        chessgames[i] = NULL;
    }

    FILE *fp = fopen("/Users/aherbrich/src/myprojects/chess/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn", "r");

    int buffersize = 8192;
    char buffer[buffersize];
    char parsedbuffer[buffersize];
    int game_counter = 0;
    int winner = 0;

    while(fgets(buffer, buffersize, fp) != NULL){
        if(strstr(buffer, "Result \"1-0\"")){
            winner = WINWHITE;
        } else if (strstr(buffer, "Result \"0-1\"")){
            winner = WINBLACK;
        } else if (strstr(buffer, "Result \"1/2-1/2\"")){
            winner = DRAW;
        }
        // if we read a newline
        if(!strcmp(buffer, "\r\n")){
            // the next line is the line with moves
            fgets(buffer, buffersize, fp);

            // parse the list of moves 
            if(strstr(buffer, "1.")){
                for(int i = 0; i < buffersize; i++){
                    parsedbuffer[i] = '\0';
                }

                // initialize chess game structure
                chessgame_t* chessgame = (chessgame_t *) malloc(sizeof(chessgame_t));
                chessgames[game_counter] = chessgame;
                // for(int i = 0; i < 4096; i++){
                //     chessgame->movelist[i] = '\0';
                // }
                game_counter++;

                chessgame->winner = winner;
                // read moves into chess game structure
                char *ptr = strtok(buffer, " ");
                int move_counter = 0;
                int idx = 0;
                while((ptr = strtok(NULL, " ")) && ptr[0]!='{'){
                    if(move_counter == 2){
                        move_counter = 0;
                        continue;
                    }
                    move_counter++;
                    strcpy(parsedbuffer+idx, ptr);
                    idx += strlen(ptr);
                    parsedbuffer[idx++] = ' ';
                }
                parsedbuffer[idx-1] = '\0';

                chessgame->movelist = (char *) malloc(strlen(parsedbuffer)+1);
                strcpy(chessgame->movelist, parsedbuffer);
                //printf("Winner:\t%d\nMoves:\t%s\n\n",chessgame->winner, chessgame->movelist);
            }

            // skip the next newline
            fgets(buffer, buffersize, fp);
        }
           
    }

    fclose(fp);
    printf("Nr. of games:\t%d\n", nr_of_games);
    return chessgames;
}

int lowercase(char character){
    return (character >= 97 && character <= 104);
}

int uppercase_piece(char character){
    return (character == 66 || character == 75 || character == 78 || character == 81 || character == 82);
}

idx_t str_to_idx(char file, char rank) {
    idx_t idx = 0;
    switch (file) {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
            idx = idx + ((file) - 'a');
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
            idx = idx + ((file) - 'A');
            break;
    }
    switch (rank) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            idx = idx + (8 * (rank - '1'));
            break;
    }
    return (idx);
}

move_t* str_to_move(board_t* board, char* token){
    // generate all possible moves in the current position
    maxpq_t movelst;
    initialize_maxpq(&movelst);
    generate_moves(board, &movelst);
    int idx = 0;

    // pawn move
    if(lowercase(token[idx])){
        char file1 = token[idx];
        char file2;
        char rank2;
        int capture = 0;
        int promotion = 0;
        char promopiece;
        
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
            rank2 = token[idx];
        }
        idx++;
        // if promotion
        if(token[idx] == '='){
            promotion = 1;
            idx++;
            promopiece = token[idx];
        }
        
        ///////////////////////
        /// FIND FITTING MOVE
        idx_t to;
        if(capture){
            to = str_to_idx(file2, rank2);
        } else{
            to = str_to_idx(file1, rank2);
        }

        for(int i = 1; i < (&movelst)->nr_elem+1; i++){
            move_t* move = (&movelst)->array[i];

            // check if move is a pawn move 
            bitboard_t from_mask = 1ULL << move->from;
            if((from_mask & board->whitepawns) || (from_mask & board->blackpawns)){
                // check if move is from the same file and to the same square
                if(to == move->to && (move->from%8+'a') == file1){
                    // if promotion
                    if(promotion){
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
                    // if non-promotion
                    else{
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                }
            } 
        }

    } else if (uppercase_piece(token[idx])){
        char file1;
        char file2;
        char rank1;
        char rank2;
        char piece = token[idx];
        int capture = 0;
        int single_ambigious = 0; // -1 if by rank, 1 if by file
        int double_ambigious = 0;

        idx++;
        if(lowercase(token[idx])){
            file1 = token[idx];
            idx++;
            if(lowercase(token[idx])){
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
                if(lowercase(token[idx+1])){
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


        ///////////////////////////
        // FIND FITTING MOVE
        idx_t to = str_to_idx(file2, rank2);

        for(int i = 1; i < (&movelst)->nr_elem+1; i++){
            move_t* move = (&movelst)->array[i];

            // check if move is a pawn move 
            bitboard_t from_mask = 1ULL << move->from;
            if(((from_mask & board->whiteknights) || (from_mask & board->blackknights)) && piece == 'N'){
                // check if move is from the same file and to the same square
                if(to == move->to){
                    if(!single_ambigious && !double_ambigious){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    else if(double_ambigious){
                        if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    } else if(single_ambigious){
                        if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    }
                }
            }
            if(((from_mask & board->whitebishops) || (from_mask & board->blackbishops)) && piece == 'B'){
                // check if move is from the same file and to the same square
                if(to == move->to){
                    if(!single_ambigious && !double_ambigious){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    else if(double_ambigious){
                        if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    } else if(single_ambigious){
                        if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    }
                }
            }
            if(((from_mask & board->whiterooks) || (from_mask & board->blackrooks)) && piece == 'R'){
                // check if move is from the same file and to the same square
                if(to == move->to){
                    if(!single_ambigious && !double_ambigious){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    else if(double_ambigious){
                        if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    } else if(single_ambigious){
                        if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    }
                }
            }
            if(((from_mask & board->whitequeens) || (from_mask & board->blackqueens)) && piece == 'Q'){
                // check if move is from the same file and to the same square
                if(to == move->to){
                    if(!single_ambigious && !double_ambigious){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    else if(double_ambigious){
                        if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    } else if(single_ambigious){
                        if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    }
                }
            }
            if(((from_mask & board->whiteking) || (from_mask & board->blackking)) && piece == 'K'){
                // check if move is from the same file and to the same square
                if(to == move->to){
                    if(!single_ambigious && !double_ambigious){
                        move_t *copy = copy_move(move);
                        free_pq(&movelst);
                        return copy;
                    }
                    else if(double_ambigious){
                        if((move->from%8+'a') == file1 && (move->from/8 + '1') == rank1){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    } else if(single_ambigious){
                        if((single_ambigious == AMBIG_BY_FILE && (move->from%8+'a') == file1) || (single_ambigious == AMBIG_BY_RANK && (move->from/8 + '1') == rank1)){
                            move_t *copy = copy_move(move);
                            free_pq(&movelst);
                            return copy;
                        }
                    }
                }
            }
        }

    } else if (token[0] == 'O'){
        int kingside;

        if(token[3]=='-'){
            kingside = 0;
        } else{
            kingside = 1;
        }

        //////////////////////////
        // FIND FITTING MOVE

        for(int i = 1; i < (&movelst)->nr_elem+1; i++){
            move_t* move = (&movelst)->array[i];
            if(kingside){
                if(move->flags == KCASTLE){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
                
            } else{
                if(move->flags == QCASTLE){
                    move_t *copy = copy_move(move);
                    free_pq(&movelst);
                    return copy;
                }
            }
        }
    }

    fprintf(stderr, "THIS SHOULD NOT HAPPEN! MOVE %s SHOULD BE POSSIBLE!\n", token);
    free_pq(&movelst);
    return NULL;
}

int play_game(chessgame_t* chessgame){
    board_t *board = init_board();
    load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    char* token = strtok(chessgame->movelist, " ");
    move_t *move = str_to_move(board, token);

    if (move){
        do_move(board, move);
        free_move(move);
    }
    else {
        print_board(board);
        fprintf(stderr, "%sInvalid move: %s%s\n",Color_PURPLE, token, Color_END);
        exit(-1);
    }

    if (token) {
        while((token = strtok(NULL, " "))) {
            move_t *move = str_to_move(board, token);
            if (move){
                do_move(board, move);
                free_move(move);
            }
            else {
                print_board(board);
                fprintf(stderr, "%sInvalid move: %s%s\n",Color_PURPLE, token, Color_END);
                exit(-1);
            }
        }
    }
    free_board(board);
    return 0;
}

int main(){
    int nr_of_games = counter_number_of_games();
    chessgame_t **chessgames = parse_chessgames_file(nr_of_games);

    // chess engine initialization
    initialize_chess_engine_only_necessary();
    initialize_zobrist_table();
    initialize_hashtable();

    // play games
    for(int i = 0; i < nr_of_games; i++){
        if(chessgames[i]){
            play_game(chessgames[i]);
        }
    }
    
}
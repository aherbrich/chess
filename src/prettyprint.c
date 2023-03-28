#include "../include/chess.h"
#include "../include/zobrist.h"

char FIELD[64][2] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
                     "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                     "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                     "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                     "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                     "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                     "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                     "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

/* Formats the string representing the board */
char* format_print_string(board_t *board){
    char *string = (char *) malloc(64);
    for(int i = 0; i < 64; i++){string[i]='-';}

    int idx;
    bitboard_t copy = board->whitepawns;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'P';
    }
    copy = board->blackpawns;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'p';
    }
    copy = board->whiteknights;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'N';
    }
    copy = board->blackknights;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'n';
    }
    copy = board->whitebishops;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'B';
    }
    copy = board->blackbishops;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'b';
    }
    copy = board->whiterooks;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'R';
    }
    copy = board->blackrooks;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'r';
    }
    copy = board->whitequeens;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'Q';
    }
    copy = board->blackqueens;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'q';
    }
    copy = board->whiteking;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'K';
    }
    copy = board->blackking;
    while((idx = pop_1st_bit(&copy)) != -1){
        string[idx] = 'k';
    }

    return string;
}

/* Print the board */
void print_board(board_t* board) {
    // create string representing the playing field
    char *board_string = format_print_string(board);

    // print the chessboard-string row for row
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }
            char piece = board_string[pos_to_idx(x, y)];
            if(piece >= 97 && piece <= 122){
                fprintf(stderr, "%s%c%s  ", Color_PURPLE, piece, Color_END);
            }
            else{
                fprintf(stderr, "%c  ", piece);
            }
            
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN, Color_END);
    fprintf(stderr, "\n");
    free(board_string);
}

/* Prints a bitboard 0/1 */
void print_bitboard(bitboard_t board){
    char *string = (char *) malloc(64);
    for(int i = 0; i < 64; i++){string[i]='-';}

    int idx;
    while((idx = pop_1st_bit(&board)) != -1){
        string[idx] = 'o';
    }

    /* print the chessboard-string row for row */
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }
            char piece = string[pos_to_idx(x, y)];
            if(piece >= 97 && piece <= 122){
                fprintf(stderr, "%s%c%s  ", Color_PURPLE, piece, Color_END);
            }
            else{
                fprintf(stderr, "%c  ", piece);
            }
            
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN, Color_END);
    fprintf(stderr, "\n");
    free(string);
}

/* Prints move on board */
void print_move_on_board(move_t *move){
    char *string = (char *) malloc(64);
    for(int i = 0; i < 64; i++){string[i]='-';}

    string[move->from] = 'o';
    if(move->flags == QUIET){
        string[move->to] = 'X';
    } else if (move->flags == CAPTURE){
        string[move->to] = 'T';
    } else if (move->flags == DOUBLEP){
        string[move->to] = 'D';
    } else if (move->flags >= 8){
        string[move->to] = 'P';
    } else if (move->flags == EPCAPTURE){
        string[move->to] = 'E';
    } else{
        string[move->to] = 'C';
    }
    

    /* print the chessboard-string row for row */
    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y == 0) {
                fprintf(stderr, "%s%d%s    ", Color_GREEN, x + 1, Color_END);
            }
            char piece = string[pos_to_idx(x, y)];
            if(piece >= 97 && piece <= 122){
                fprintf(stderr, "%s%c%s  ", Color_PURPLE, piece, Color_END);
            }
            else{
                fprintf(stderr, "%c  ", piece);
            }
            
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n     %sA  B  C  D  E  F  G  H%s", Color_GREEN, Color_END);
    fprintf(stderr, "\n");
    free(string);
}

/* Print move */
void print_move(move_t* move) {
    char* start_field = FIELD[move->from];
    char* end_field = FIELD[move->to];
    if (move->flags >= 8) {
        fprintf(stderr, "%s%.2s-%.2s-P%s",Color_PURPLE, start_field, end_field,Color_END);

    } else if (move->flags == KCASTLE || move->flags == QCASTLE) {
        fprintf(stderr, "%s%.2s-%.2s-C%s", Color_PURPLE, start_field, end_field, Color_END);
    } else {
        fprintf(stderr, "%s%.2s-%.2s%s",Color_PURPLE, start_field, end_field, Color_END);
    }
}

/* Print move */
void print_LAN_move(move_t* move, player_t color_playing) {
    char* start_field = FIELD[move->from];
    char* end_field = FIELD[move->to];

    /* if promotion move */
    if (move->flags >= 8) {
        flag_t prom_flag = move->flags & ~(0b11);
        if(prom_flag == 0){
            if(color_playing == WHITE){
                printf("%.2s%.2sN", start_field, end_field);
            }else{
                printf("%.2s%.2sn", start_field, end_field);
            }
        }
        else if(prom_flag == 1){
            if(color_playing == WHITE){
                printf("%.2s%.2sB", start_field, end_field);
            }else{
                printf("%.2s%.2sb", start_field, end_field);
            }
        }
        else if(prom_flag == 2){
            if(color_playing == WHITE){
                printf("%.2s%.2sR", start_field, end_field);
            }else{
                printf("%.2s%.2sr", start_field, end_field);
            }
        }
        else{
            if(color_playing == WHITE){
                printf("%.2s%.2sQ", start_field, end_field);
            }else{
                printf("%.2s%.2sq", start_field, end_field);
            }
        }
    } 
    /* no promotion move */
    else{
        printf("%.2s%.2s", start_field, end_field);
    }
}

/* Prints all moves in a movelist */
void print_movelist(list_t* movelst){
    move_t *move;
    while((move = pop(movelst))){
        print_move(move);
        fprintf(stderr, " ");
        free_move(move);
    }
    free_move_list(movelst);
    fprintf(stderr, "\n");
}

/* Print the list of best possible moves until a depth (PV line) */
void print_line(board_t* board, int depth) {
    /* make a copy of the board */
    board_t *board_copy = copy_board(board);

    /* for all depth, figure out the best move from the hash-table and print it */
    for (int d = depth; d > 0; d--) {
        move_t* best_move = get_best_move_from_hashtable(board_copy);
        if (best_move == NULL) {
            printf("NULL ");
            free_board(board_copy);
            return;
        }
        print_LAN_move(best_move, board_copy->player);
        printf(" ");
        do_move(board_copy, best_move);
        free_move(best_move);
    }

    /* free the board */
    free_board(board_copy);

    return;
}
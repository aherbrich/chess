#include <stdio.h>

#include "include/engine-core/engine.h"

tt_t tt;

int main(void) {
    /* seed random number generator */
    srand(0);

    /* initialize zobrist table */
    initialize_zobrist_table();

    /* intialize board */
    board_t* board = init_board();
    load_by_FEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");


    /* initialize transposition table */
    tt = init_tt(MB_TO_BYTES(1024));

    /* check if transposition table is initialized correctly */
    for(int i = 0; i < tt.size; i++){
        if(tt.buckets[i].always_replace.key != 0ULL || tt.buckets[i].replace_if_better.key != 0ULL){
            printf("%sFAIL%s: transposition table is not initialized correctlly \n", Color_RED, Color_END);
            exit(EXIT_FAILURE);  
        } 
    }
    printf("%sSUCCESS%s: transposition table is initialized correctly - size %d (%dMb)) - bits %d\n", Color_GREEN, Color_END, tt.size, (int) BYTES_TO_MB(tt.size*sizeof(tt_bucket_t)), tt.no_bits);


    /* check if RETREIVE and STORE are working correctly */

    /* (1) check if entry is NULL for board for which no entry should exist */
    tt_entry_t* entry_null = retrieve_tt_entry(tt, board);
    if(entry_null == NULL){
        printf("%sSUCCESS%s: no entry for board\n", Color_GREEN, Color_END);
    } else {
        printf("%sFAIL%s: entry for board\n",Color_RED, Color_END);
        exit(EXIT_FAILURE);
    }

   
    /* add entry for board (with 'move_one' as best move) to transposition table */
    move_t move_one = {.value = 0, .from = 8, .to = 16, .flags = 0};
    store_tt_entry(tt, board, move_one, 5, 100, EXACT);

    /* (2.1) check if we find entry for board in transposition table */
    tt_entry_t* entry = retrieve_tt_entry(tt, board);
    if(entry != NULL && is_same_move(&entry->best_move, &move_one) && entry->depth == 5 && entry->eval == 100 && entry->flags == EXACT){
        printf("%sSUCCESS%s: test 1: entry is in transposition table\n", Color_GREEN, Color_END);
    } else {
        printf("%sFAIL%s: test 1: entry is not in transposition table\n", Color_RED, Color_END);
        exit(EXIT_FAILURE);
    } 

    /* add entry for board (with 'move_two' as best move) to transposition table */
    move_t move_two = {.value = 0, .from = 8, .to = 16, .flags = 1};
    store_tt_entry(tt, board, move_two, 4, 200, LOWERBOUND);

    /* (2.2) check if we find entry for board in transposition table */
    /* (2.2) AND! that we retrieve move_one since it has higher depth */
    entry = retrieve_tt_entry(tt, board);
    if(entry != NULL && is_same_move(&entry->best_move, &move_one) && entry->depth == 5 && entry->eval == 100 && entry->flags == EXACT){
        printf("%sSUCCESS%s: test 2: entry is in transposition table\n", Color_GREEN, Color_END);
    } else {
        printf("%sFAIL%s: test 2: entry is not in transposition table\n", Color_RED, Color_END);
        exit(EXIT_FAILURE);
    }

    /* add entry for board (with 'move_three' as best move) to transposition table */
    move_t move_three = {.value = 0, .from = 8, .to = 16, .flags = 2};
    store_tt_entry(tt, board, move_three, 6, 300, UPPERBOUND);
    
    
    /* (2.3) check if we find entry for board in transposition table */
    /* (2.3) AND! that we retrieve move_three since it has higher depth */
    entry = retrieve_tt_entry(tt, board);
    if(entry != NULL && is_same_move(&entry->best_move, &move_three) && entry->depth == 6 && entry->eval == 300 && entry->flags == UPPERBOUND){
        printf("%sSUCCESS%s: test 3: entry is in transposition table\n", Color_GREEN, Color_END);
    } else {
        printf("%sFAIL%s: test 3: entry is not in transposition table\n", Color_RED, Color_END);
        exit(EXIT_FAILURE);
    }
}

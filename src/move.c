#include "../include/chess.h"

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS


/* Allocate memory for a move */
move_t *generate_move(idx_t from, idx_t to, flag_t flags) {
    move_t *move = (move_t *)malloc(sizeof(move_t));

    move->from = from;
    move->to = to;
    move->flags = flags;

    return move;
}

/* Copy a move */
move_t *copy_move(move_t *move) {
    if (move == NULL) {
        return NULL;
    }

    move_t *copy = (move_t *) malloc(sizeof(move_t));
    
    copy->from = move->from;
    copy->to = move->to;
    copy->flags = move->flags;

    return copy;
}

/* Frees memory of move */
void free_move(move_t *move) {
    if (move) {
        free(move);
    }
}

/* Frees memory of move list */
void free_move_list(node_t *movelst) {
    move_t *move;
    while ((move = pop(movelst)) != NULL) {
        free_move(move);
    }

    if(movelst)free(movelst);
}
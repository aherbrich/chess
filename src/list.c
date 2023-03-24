#include "../include/chess.h"

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

/* Allocate memory for new list */
list_t* new_list() {
    list_t* head = (list_t*) malloc(sizeof(list_t));
    head->len = 0;
    head->first = NULL;
    head->last = NULL;

    return head;
}

/* Pushes a move node into list (as the last element of list)*/
void push(list_t* head, move_t *move){
    node_t* node = (node_t*) malloc(sizeof(node_t));
    node->move = move;
    node->board = NULL;
    node->next = NULL;
    node->prev = NULL;

    // if list is empty
    if(!head->last){
        head->first = node;
        head->last = node;
        head->len++;
    }
    // if list is not empty
    else{
        node->prev = head->last;

        head->last->next = node;
        head->last = node;
        head->len++;
    } 
}

/* Pops a move node from list (the last element of list)*/
move_t* pop(list_t* head){
    // if non valid list
    if(!head){
        fprintf(stderr, "Tried to pop element from NULL list...ERROR!\n");
        exit(1);
    }

    // if empty list
    else if(head->len == 0){
        return NULL;
    }

    // if one element list
    else if(head->len == 1){
        move_t* tmp_move = head->last->move;
        node_t* tmp_node = head->last;

        head->first = NULL;
        head->last = NULL;
        head->len--;

        free(tmp_node);
        return (tmp_move);
    }

    // if more element list
    else {
        move_t* tmp_move = head->last->move;
        node_t* tmp_node = head->last;

        head->last->prev->next = NULL;
        head->last = head->last->prev;
        head->len--;

        free(tmp_node);
        return (tmp_move);
    }
}
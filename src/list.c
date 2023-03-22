#include "../include/chess.h"

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

/*
node_t* init_list() {
    node_t* head = (node_t*) malloc(sizeof(node_t));
    head->move = NULL;
    head->next = NULL;
    return (head);
}

void add(node_t* head, move_t* move) {
    node_t* node = (node_t*) malloc(sizeof(node_t));
    node->move = move;
    node->next = NULL;

    node_t* ptr = head;

    while (ptr->next != NULL) {
        ptr = ptr->next;
    }

    ptr->next = node;
}

void delete(node_t* head) {
    node_t* ptr = head;
    node_t* prev;

    // find last element
    while (ptr->next != NULL) {
        prev = ptr;
        ptr = ptr->next;
    }

    // if last element is head element
    if (ptr == head) {
        // do nothing
        return;
    }

    // set pointer correctly
    prev->next = NULL;

    // free memory
    free(ptr->move);
    free(ptr);
}

move_t* pop(node_t* head) {
    node_t* ptr = head;
    node_t* prev;

    // find last element
    while (ptr->next != NULL) {
        prev = ptr;
        ptr = ptr->next;
    }

    // if last element is head element
    if (ptr == head) {
        // do nothing
        return NULL;
    }

    // set pointer correctly
    prev->next = NULL;

    // free memory
    move_t* tmp = ptr->move;
    free(ptr);

    return (tmp);
}

int len(node_t* head) {
    node_t* ptr = head->next;
    int counter = 0;

    while (ptr != NULL) {
        counter++;
        ptr = ptr->next;
    }

    return counter;
}
*/

list_t* new_list() {
    list_t* head = (list_t*) malloc(sizeof(list_t));
    head->len = 0;
    head->first = NULL;
    head->last = NULL;

    return head;
}

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


void push_old_state(list_t* head, board_t *board){
    node_t* node = (node_t*) malloc(sizeof(node_t));
    node->move = NULL;
    node->board = copy_board(board);
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

board_t* pop_old_state(list_t* head){
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
        board_t* tmp_board = head->last->board;
        node_t* tmp_node = head->last;

        head->first = NULL;
        head->last = NULL;
        head->len--;

        free(tmp_node);
        return (tmp_board);
    }

    // if more element list
    else {
        board_t* tmp_board = head->last->board;
        node_t* tmp_node = head->last;

        head->last->prev->next = NULL;
        head->last = head->last->prev;
        head->len--;

        free(tmp_node);
        return (tmp_board);
    }
}
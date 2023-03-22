#include "../include/chess.h"

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

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
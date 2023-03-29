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
void push(list_t* head, move_t* move){
    node_t* node = (node_t*) malloc(sizeof(node_t));
    node->move = move;
    node->next = NULL;
    node->prev = NULL;

    // if list is empty
    if(!head->last){
        head->first = node;
        head->last = node;
        head->len++;
        return;
    }
    // if list is not empty
    /* if move is a promotion start from end of list (since it will likely be one of the last)*/
    if(move->flags & 0b1000){
        /* (1) either we add the move between/infront of existing nodes */
        node_t* ptr = head->last;
        while(ptr != NULL){
            if(ptr->move->value < move->value){
                head->len++;
                node->next = ptr->next;
                node->prev = ptr;
                ptr->next = node;
                if(node->next == NULL){
                    head->last = node;
                }
                return;
            }
        }

        /* (2) or we add the move at the last place in the list */
        node->next = head->first;
        head->first->prev = node;
        head->first = node;
        head->len++;
    }
    /* else start from beginning of list */
    else{
        /* (1) either we add the move between/infront of existing nodes */
        node_t* ptr = head->first;
        while(ptr != NULL){
            if(ptr->move->value >= move->value){
                head->len++;
                node->prev = ptr->prev;
                node->next = ptr;
                ptr->prev = node;
                if(node->prev == NULL){
                    head->first = node;
                }
                return;
            }
            ptr = ptr->next;
        }

        /* (2) or we add the move at the last place in the list */
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
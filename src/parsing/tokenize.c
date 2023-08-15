#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/parse/parse.h"

/* allocates memory for token iterator*/
token_iterator_t* new_token_iterator(){
    return (token_iterator_t *) malloc(sizeof(token_iterator_t));
};

/* frees memory for token iterator (and it's fields) */
void delete_token_iterator(token_iterator_t* it){
    if(it->input) free(it->input);
    if(it->delimiters) free(it->delimiters);
    if(it->token) free(it->token);
    free(it);
}

/* initializes token iterator */
token_iterator_t* tokenize(token_iterator_t* it, char *input, char* delimiters){
    if(it == NULL || input == NULL || delimiters == NULL){
        fprintf(stderr, "Invalid call! it:%p in:%p del:%p\n", it, input, delimiters);
        exit(1);
    }

    /* copy input */
    it->delimiters = (char*) malloc(strlen(delimiters)+1);
    strncpy(it->delimiters, delimiters, strlen(delimiters)+1);

    /* copy delimiters */
    it->input = (char*) malloc(strlen(input)+1);
    strncpy(it->input, input, strlen(input)+1);

    /* skip leading delimiters */
    it->next = it->input +  strspn(it->input, it->delimiters);

    /* get span until first occurence of delimiter */
    int length = strcspn (it->next, it->delimiters);
    /* if length is 0, we are done */
    if(length == 0){
        it->token = NULL;
        it->next = NULL;
        return it;
    } 
    /* if we read the last token (indicated by EOF), we are done */
    else if(it->next[length] == '\0'){
        it->token = (char *) malloc(length);
        strncpy(it->token, it->next, length);
        it->next = NULL;
    } 
    /* else we still have tokens to read */
    else{
        it->token = (char *) malloc(length+1);
        strncpy(it->token, it->next, length);
        it->token[length] = '\0';
        it->next += length;
    }

    return it;
}

/* returns 1 if there are tokens left */
int token_left(token_iterator_t* it){
    if(it->token){
        return 1;
    } else{
        return 0;
    }
}

/* extracts the next token */
void token_next(token_iterator_t* it){
    /* free token from last iteration */
    free(it->token);
    
    /* if we have nothing to read, indicate we are done iterating */
    if(it->next == NULL){
        it->token = NULL;
        return;
    }
    
    /* ELSE */
    /* skip leading delimiters */
    it->next += strspn (it->next, it->delimiters);
    /* get span until first occurence of delimiter */
    int length = strcspn (it->next, it->delimiters);

    /* if length is 0, we are done */
    if(length == 0){
        it->token = NULL;
        it->next = NULL;
        return;
    } 
    /* if we read the last token (inidcated by EOF), we are done */
    else if(it->next[length] == '\0'){
        it->token = (char *) malloc(length);
        strncpy(it->token, it->next, length);
        it->next = NULL;
    } 
    /* else we still have tokens to read */
    else{
        it->token = (char *) malloc(length+1);
        strncpy(it->token, it->next, length);
        it->token[length] = '\0';
        it->next += length;
    }
}
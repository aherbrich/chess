#ifndef __URGENCIES_H__
#define __URGENCIES_H__

#include "include/ordering/gaussian.h"

#define HT_GAUSSIAN_SIZE (1048576)
#define HASH_SIZE (20)
#define MAX_MOVES 512

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the hash-table of a urgencies for the Bayesian move ranking model         */
/* ------------------------------------------------------------------------------------------------ */
typedef struct _urgency_ht_list_entry_t {
    int move_key;                          /* true (non-clashing) value of the move */
    gaussian_t urgency;                    /* urgency of the move */
    struct _urgency_ht_list_entry_t* next; /* pointer to the next entry in the list */
} urgency_ht_list_entry_t;

typedef struct _urgency_ht_entry_t {
    urgency_ht_list_entry_t* root; /* pointer to the root of the list of urgencies for this move */
} urgency_ht_entry_t;

typedef struct _urgency_ht_iterator_t {
    int cur_hash;                        /* current hash value */
    urgency_ht_list_entry_t* cur_urgency_entry; /* pointer to the current entry in the list */
} urgency_ht_iterator_t;

/* hash table of urgencies for each move (hash) */
extern urgency_ht_entry_t* ht_urgencies;

/* initializes an urgency hashtable with empty cells */
urgency_ht_entry_t* initialize_ht_urgencies();
/* retrieves a pointer to the urgency belief for a given move key and move hash */
gaussian_t* get_urgency(urgency_ht_entry_t* ht, int move_key);
/* adds a (Gaussian) urgency belief to the hash table */
gaussian_t* add_urgency(urgency_ht_entry_t* ht, int move_key, gaussian_t urgency);
/* returns the number of keys in the urgency hash table */
int get_no_keys(const urgency_ht_entry_t* ht);
/* deletes the memory for a hashtable of urgencies */
void deletes_ht_urgencies(urgency_ht_entry_t* ht);
/* loads a hash-table of urgencies from a file (only entries which are different from the prior) */
void load_ht_urgencies_from_binary_file(const char* file_name, urgency_ht_entry_t* ht);
/* writes an urgencies hash-table to a file (only entries which are different from the prior) */
void write_ht_urgencies_to_binary_file(const char* file_name, const urgency_ht_entry_t* ht);
/* checks if two hash-tables are equivalent */
int ht_urgencies_equal(urgency_ht_entry_t* ht1, urgency_ht_entry_t* ht2);

/* sets up an urgency hash-table iterator */
void setup_ht_urgencies_iterator(const urgency_ht_entry_t* ht, urgency_ht_iterator_t* it);
/* increments the urgency hash-table iterator */
void inc_ht_urgencies_iterator(const urgency_ht_entry_t* ht, urgency_ht_iterator_t* it);
/* checks if the iterator is at the end */
int ht_urgencies_iterator_finished(const urgency_ht_iterator_t* it);

#endif
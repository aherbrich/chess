#include <stdio.h>
#include <stdlib.h>

#include "include/ordering/urgencies.h"

#include "include/engine-core/engine.h"
#include "include/ordering/gaussian.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for managing the hash-table of a urgencies for the Bayesian move ranking model         */
/* ------------------------------------------------------------------------------------------------ */

/* hash table of urgencies for each move (hash) */
urgency_ht_entry_t* ht_urgencies = 0;

/* initializes an urgency hashtable with standard Normals */
urgency_ht_entry_t* initialize_ht_urgencies() {
    urgency_ht_entry_t* ht = (urgency_ht_entry_t*)malloc(sizeof(urgency_ht_entry_t) * HT_GAUSSIAN_SIZE);
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        ht[i].root = NULL;
    }
    return ht;
}

/* deletes the memory for a hashtable of urgencies */
void deletes_ht_urgencies(urgency_ht_entry_t* ht) {
    if (ht) {
        /* free all linked list for entries where there was a clash */
        for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
            /* free the linked list, if there is no single key */
            while (ht[i].root) {
                urgency_ht_list_entry_t* next = ht[i].root->next;
                free(ht[i].root);
                ht[i].root = next;
            }
        }

        /* then free the whole hash table */
        free(ht);
    }
    return;
}

/* function that computes the move hash using a multiplicative hash */
int move_hash(int move_key) {
    uint32_t knuth = 2654435769;
    uint32_t y = move_key;
    int move_hash = (y * knuth) >> (32 - HASH_SIZE);
    return move_hash;
}

/* retrieves a pointer to the urgency belief for a given move key and move hash */
gaussian_t* get_urgency(urgency_ht_entry_t* ht, int move_key) {
    int hash = move_hash(move_key);

    /* otherwise search the linked list for the move key */
    urgency_ht_list_entry_t* entry = ht[hash].root;
    while (entry) {
        if (entry->move_key == move_key) {
            return &entry->urgency;
        }
        entry = entry->next;
    }

    /* if that cannot be found, then return NULL */
    return NULL;
}

/* adds a (Gaussian) urgency belief to the hash table */
gaussian_t* add_urgency(urgency_ht_entry_t* ht, int move_key, gaussian_t urgency) {
    int hash = move_hash(move_key);

    urgency_ht_list_entry_t* entry = (urgency_ht_list_entry_t*)malloc(sizeof(urgency_ht_list_entry_t));
    entry->move_key = move_key;
    entry->urgency = urgency;
    entry->next = ht[hash].root;
    ht[hash].root = entry;
    return &entry->urgency;
}

/* returns the number of keys in the urgency hash table */
int get_no_keys(const urgency_ht_entry_t* ht) {
    int cnt = 0;
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        urgency_ht_list_entry_t* entry = ht[i].root;
        while (entry) {
            cnt++;
            entry = entry->next;
        }
    }
    return cnt;
}

/* a unique cookie representing the file version */
int file_version_cookie = 0x20;

/* writes an urgencies hash-table to a file */
void write_ht_urgencies_to_binary_file(const char* file_name, const urgency_ht_entry_t* ht) {
    FILE* fp = fopen(file_name, "wb");
    if (fp != NULL) {
        fwrite(&file_version_cookie, sizeof(int), 1, fp);
        for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
            urgency_ht_list_entry_t* entry = ht[i].root;
            while (entry) {
                fwrite(&entry->move_key, sizeof(int), 1, fp);
                fwrite(&entry->urgency, sizeof(gaussian_t), 1, fp);
                entry = entry->next;
            }
        }
    }
    fclose(fp);
}

/* loads a hash-table of urgencies from a file (only entries which are different from the prior) */
void load_ht_urgencies_from_binary_file(const char* file_name, urgency_ht_entry_t* ht) {
    FILE* fp = fopen(file_name, "rb");
    if (fp != NULL) {
        int cookie;
        fread(&cookie, sizeof(int), 1, fp);

        if (cookie != file_version_cookie) {
            fprintf(stderr, "Error: file version mismatch\n");
            exit(1);
        }

        int move_key;
        gaussian_t gaussian;
        while (fread(&move_key, sizeof(int), 1, fp) == 1) {
            fread(&gaussian, sizeof(gaussian_t), 1, fp);
            add_urgency(ht, move_key, gaussian);
        }
    }
    fclose(fp);
}

/* checks if two hash-tables are equivalent */
int ht_urgencies_equal(urgency_ht_entry_t* ht1, urgency_ht_entry_t* ht2) {
    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        /* check if each key in ht1 is also contained and equivalent to ht2 */
        urgency_ht_list_entry_t* entry1 = ht1[i].root;
        while (entry1) {
            gaussian_t* g_ptr;

            if ((g_ptr = get_urgency(ht2, entry1->move_key)) == NULL) {
                return 0;
            } else if (g_ptr->rho != entry1->urgency.rho || g_ptr->tau != entry1->urgency.tau) {
                return 0;
            }
            entry1 = entry1->next;
        }

        /* check if each key in ht1 is also contained and equivalent to ht2 */
        urgency_ht_list_entry_t* entry2 = ht2[i].root;
        while (entry2) {
            gaussian_t* g_ptr;

            if ((g_ptr = get_urgency(ht1, entry2->move_key)) == NULL) {
                return 0;
            } else if (g_ptr->rho != entry2->urgency.rho || g_ptr->tau != entry2->urgency.tau) {
                return 0;
            }
            entry2 = entry2->next;
        }
    }
    return 1;
}

/* sets up an urgency hash-table iterator */
void setup_ht_urgencies_iterator(const urgency_ht_entry_t* ht, urgency_ht_iterator_t* it) {
    it->cur_hash = -1;
    it->cur_urgency_entry = NULL;

    for (int i = 0; i < HT_GAUSSIAN_SIZE; i++) {
        if (ht[i].root) {
            it->cur_hash = i;
            it->cur_urgency_entry = ht[i].root;
            return;
        }
    }
    return;
}

/* increments the urgency hash-table iterator */
void inc_ht_urgencies_iterator(const urgency_ht_entry_t* ht, urgency_ht_iterator_t* it) {
    if (it->cur_urgency_entry->next) {
        it->cur_urgency_entry = it->cur_urgency_entry->next;
    } else {
        for (int i = it->cur_hash + 1; i < HT_GAUSSIAN_SIZE; i++) {
            if (ht[i].root) {
                it->cur_hash = i;
                it->cur_urgency_entry = ht[i].root;
                return;
            }
        }
        it->cur_hash = -1;
        it->cur_urgency_entry = NULL;
    }

    return;
}

/* checks if the iterator is at the end */
int ht_urgencies_iterator_finished(const urgency_ht_iterator_t* it) {
    return it->cur_hash == -1;
}
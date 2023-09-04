#ifndef __UCI_H__
#define __UCI_H__

#include "include/engine-core/types.h"
#include "include/engine-core/search.h"

/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for option and engine info handling                                        */
/* ------------------------------------------------------------------------------------------------ */


/* engine info */
typedef struct _engine_info_t {
    char *name;
    char *author;
    char *version;
} engine_info_t;

typedef struct _spin_value_t {
    int min;
    int max;
    int def;
} spin_value_t;

/* chess engine options */
typedef struct _options_t {
    spin_value_t opt_hash; 
} options_t;

options_t init_options(void);
engine_info_t init_engine_info(void);


/* ------------------------------------------------------------------------------------------------ */
/* structs and functions for uci interface                                                          */
/* ------------------------------------------------------------------------------------------------ */

typedef struct _uci_args_t {
    board_t* board;
    searchdata_t* searchdata;
    engine_info_t engine_info;
    options_t options;
    int verbosity_level;
} uci_args_t;

/* starts the uci interface */
void uci_interface_loop(void *args);

#endif

#include "include/engine-core/tt.h"

/* ------------------------------------------------------------------------------------------------ */
/* functions for initialization of chess engine                                                     */
/* ------------------------------------------------------------------------------------------------ */

void initialize_chess_engine_necessary();
void initialize_zobrist_table();
tt_t init_tt(int size_in_bytes);
void initialize_attack_boards();
void initialize_helper_boards();
#ifndef __CHESS_H__
#define __CHESS_H__

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define Color_YELLOW "\033[0;33m"
#define Color_GREEN "\033[0;32m"
#define Color_RED "\033[0;31m"
#define Color_CYAN "\033[0;36m"
#define Color_PURPLE "\033[0;35m"
#define Color_WHITE "\033[0;37m"
#define Color_END "\033[0m"

#define WHITE 0
#define BLACK 1

#define EMPTY 0
#define PAWN 2
#define KNIGHT 4
#define BISHOP 8
#define ROOK 16
#define QUEEN 32
#define KING 64

#define NORMALMOVE 1
#define CASTLEMOVE 2
#define ENPASSANTMOVE 4
#define PROMOTIONMOVE 8
#define EPPOSSIBLEMOVE 16

#define LONGSIDEW 1
#define SHORTSIDEW 2
#define LONGSIDEB 4
#define SHORTSIDEB 8

#define TRUE 1
#define FALSE 0

#define COLOR(X) ((X)&1)
#define FIGURE(X) ((X) & ~1)
#define OPPONENT(X) ((X == 0) ? (1) : (0))

#define INFINITY INT_MAX
#define NEGINFINITY (-INFINITY)

#define MAXIMIZING(X) ((X == 0) ? (1) : (0))

#define FLG_EXCACT 1
#define FLG_CUT 2
#define FLG_ALL 4

#define ND_EXACT(X) ((X & 1) != 0)
#define ND_CUT(X) ((X & 2) != 0)
#define ND_ALL(X) ((X & 4) != 0)

#define MAXNR_LINES 200
#define MAXDEPTH_LINE 40

typedef uint8_t piece_t;
typedef uint8_t player_t;
typedef uint8_t flag_t;
typedef uint8_t dir_t;
typedef int8_t idx_t;
typedef uint16_t order_t;

typedef struct _board_t {
    piece_t* playing_field;
    player_t player;
    flag_t castle_rights;
    flag_t ep_possible;
    piece_t ep_field;
    uint16_t ply_no;
} board_t;

typedef struct _oldflags_t {
    flag_t castle_rights;
    flag_t ep_possible;
    piece_t ep_field;
} oldflags_t;

typedef struct _move_t {
    flag_t type_of_move;

    piece_t piece_was;
    piece_t piece_is;
    piece_t piece_cap;

    idx_t start;
    idx_t end;
    idx_t optional_start;
    idx_t optional_end;

    flag_t new_cr;
    flag_t new_ep;

    oldflags_t* oldflags;

    order_t order_id;
} move_t;

typedef struct _node_t {
    move_t* move;
    struct _node_t* next;
} node_t;

typedef struct _search_data {
    board_t *board;         /* pointer to the actual board */
    int max_depth;          /* maximum search depth in plies */
    int max_seldepth;       /* maximum search depth with quiescence search */
    int max_nodes;          /* maximum nodes allowed to search */
    int max_time;           /* maximum time allowed */
    int wtime;              /* time white has left on clock in ms*/
    int btime;              /* time black has left on clock in ms*/
    int winc;               /* white time increment in ms */      
    int binc;               /* black time increment in ms */ 
    int ponder;             /* tells engine to start search at ponder move */
    int run_infinite;       /* tells the engine to run aslong as stop != 1 */
    int stop;               /* tells the engine to stop search when stop == 1*/
    move_t *best_move;      /* computed best move (so far) */
    clock_t start_time;     /* time the search was initiated by gui */
    int time_available;     /* time for search precalculated */
}  search_data;

/////////////////////////////////////////////////////////////
//  GLOBAL PERFORMANCE COUNTER

extern int nodes_searched;
extern int hash_used;
extern int hash_bounds_adjusted;

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

/* Allocate memory for list */
extern node_t* init_list();
/* Determine length of list */
extern int len(node_t* head);
/* List helpers*/
extern void add(node_t* head, move_t* move);
extern move_t* pop(node_t* head);

////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

/* Allocate memory for a specific move */
extern move_t* create_normal_move(piece_t piece_was, piece_t piece_cap, idx_t start, idx_t end, flag_t new_cr, oldflags_t* oldflags);
extern move_t* create_ep_possible_move(piece_t piece_was, idx_t start, idx_t end, flag_t new_ep_field, oldflags_t* oldflags);
extern move_t* create_promotion_move(piece_t piece_was, piece_t piece_is, piece_t piece_cap, idx_t start, idx_t end, flag_t new_cr, oldflags_t* oldflags);
extern move_t* create_castle_move(idx_t start_king, idx_t end_king, idx_t start_rook, idx_t end_rook, flag_t new_cr, oldflags_t* oldflags);
extern move_t* create_ep_move(idx_t start_attacker, idx_t end_attacker, oldflags_t* oldflags);
/* Frees memory of move */
extern void free_move(move_t* move);
/* Frees memory of move list */
extern void free_move_list(node_t* move_list);
/* Deep copies move */
extern move_t* copy_move(move_t* move);
/* Execute move */
extern void play_move(board_t* board, move_t* move, player_t player_who_made_move);
/* Reverses a move/ recovers old board state */
extern void reverse_move(board_t* board, move_t* move, player_t player_who_made_move);
/* Sorts list by capture order */
extern node_t* sort_moves(node_t* head);
/* Returns true if two moves are identical */
extern int is_same_move(move_t* move, move_t* move2);
/* Converts a string to a move in the context of an existing board */
extern move_t *str_to_move(board_t *board, char *move_str);

///////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

/* Clears the board */
extern void clear_board(board_t *board);
/* Allocate memory for a empty board */
extern board_t* init_board();
/* Make deep cop of board */
extern board_t* copy_board(board_t* board);
/* Free memory of board */
extern void free_board(board_t* board);
/* Load a game position based on FEN */
extern void load_by_FEN(board_t* board, char* FEN);

/////////////////////////////////////////////////////////////
// PERFT TESTER

/* Perft move generation and validation */
extern int move_gen(board_t* board, int depth);

//////////////////////////////////////////////////////////////
//  USEFUL FUNCTIONS

/* Calculate idx based on a row & column */
extern idx_t pos_to_idx(int row, int col);
/* Copies flags from board */
extern oldflags_t* copy_flags(board_t* board);
/* Copies flags from move */
extern oldflags_t* copy_flags_from_move(move_t* move);
/* Max of function */
extern int max(int x, int y);
/* Min of function */
extern int min(int x, int y);

//////////////////////////////////////////////////////////////
//  MOVE GENERATION

/* Move generation */
extern node_t* generate_moves(board_t* board);
/* Generates all captures */
extern node_t* generate_captures(board_t* board);
/* Checks if a move is legal (king not in check) */
extern int is_legal_move(board_t* board);
extern int PVMove_is_possible(node_t* movelst, move_t* ttmove);

///////////////////////////////////////////////////////////////
//  SEARCH

/* alpha beta search with transposition table */
extern int alphaBeta_with_TT(board_t* board, uint8_t depth, int alpha, int beta, search_data *data);
/* iterative search */
extern move_t* iterative_search(search_data *data);
/* allocates memory for a search data structure */
extern search_data* init_search_data(board_t *board);
/* frees search data */
extern void free_search_data(search_data *data);

#endif
#ifndef __CHESS_H__
#define __CHESS_H__

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#define Color_YELLOW "\033[0;33m"
#define Color_GREEN "\033[0;32m"
#define Color_RED "\033[0;31m"
#define Color_CYAN "\033[0;36m"
#define Color_PURPLE "\033[0;35m"
#define Color_WHITE "\033[0;37m"
#define Color_END "\033[0m"

#define WHITE 1
#define BLACK 0

#define TRUE 1
#define FALSE 0

#define LONGSIDEW 1
#define SHORTSIDEW 2
#define LONGSIDEB 4
#define SHORTSIDEB 8

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6
#define H 7

#define RANK1 0
#define RANK2 1
#define RANK3 2
#define RANK4 3
#define RANK5 4
#define RANK6 5
#define RANK7 6
#define RANK8 7

#define QUIET 0
#define DOUBLEP 1
#define KCASTLE 2
#define QCASTLE 3
#define CAPTURE 4
#define EPCAPTURE 5
#define KPROM 8
#define BPROM 9
#define RPROM 10
#define QPROM 11
#define KCPROM 12
#define BCPROM 13
#define RCPROM 14
#define QCPROM 15

#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

#define INFINITY INT_MAX
#define NEGINFINITY (-INFINITY)

#define EXACT 0
#define UPPERBOUND 1
#define LOWERBOUND 2

#define PRIORITY_QUEUE_SIZE 512

#define WINWHITE 1
#define DRAW 0
#define WINBLACK -1

typedef uint64_t bitboard_t;
typedef uint8_t player_t;
typedef uint8_t idx_t;
typedef uint8_t flag_t;
typedef uint8_t piece_t;

typedef struct _board_t {
    bitboard_t whitepawns;
    bitboard_t whiteknights;
    bitboard_t whitebishops;
    bitboard_t whiterooks;
    bitboard_t whitequeens;
    bitboard_t whiteking;
    bitboard_t blackpawns;
    bitboard_t blackknights;
    bitboard_t blackbishops;
    bitboard_t blackrooks;
    bitboard_t blackqueens;
    bitboard_t blackking;
    bitboard_t black;
    bitboard_t white;
    bitboard_t all;

    player_t player;
    flag_t castle_rights;
    flag_t ep_possible;
    idx_t ep_field;
    uint16_t ply_no;
    uint16_t full_move_counter;
    uint8_t fifty_move_counter;
} board_t;

typedef struct _move_t {
    uint16_t value;
    idx_t from;
    idx_t to;
    flag_t flags;
} move_t;
typedef struct _node_t {
    move_t* move;
    struct _node_t* next;
    struct _node_t* prev;
} node_t;

typedef struct _list_t{
    int len;
    node_t *first;
    node_t *last;
} list_t;

typedef struct _maxpq_t{
    int size;
    int nr_elem;
    move_t* array[PRIORITY_QUEUE_SIZE];
} maxpq_t;
typedef struct _searchdata_t {
    board_t *board;                 // pointer to the actual board 
    int max_depth;                  // maximum search depth in plies
    int max_seldepth;               // maximum search depth with quiescence search 
    int max_nodes;                  // maximum nodes allowed to search 
    int max_time;                   // maximum time allowed 
    int wtime;                      // time white has left on clock in ms
    int btime;                      // time black has left on clock in ms
    int winc;                       // white time increment in ms   
    int binc;                       // black time increment in ms 
    int ponder;                     // tells engine to start search at ponder move 
    int run_infinite;               // tells the engine to run aslong as stop != 1
    int stop;                       // tells the engine to stop search when stop == 1
    int time_available;             // tells the engine how much time it has to search in ms

    struct timeval start;
    struct timeval end;
    move_t *best_move;              // best move (so far) 
    int best_eval;                  // evaluation of board after best move made
    int nodes_searched;             // amount of nodes searched in iterative search
    int hash_used;                  // amount of hash entries that lead to not needing to search the node again 
    int hash_bounds_adjusted;       // amount of hash entries that lead to a adjustment of alpha/beta bounds
    int pv_node_hit;                // amount of pv moves that turned out to be the best move
}  searchdata_t;

typedef struct _chessgame_t{
    char* movelist;
    int winner;
} chessgame_t;

/////////////////////////////////////////////////////////////
//  GLOBALS 

//  SEARCH
extern int nodes_searched;
extern int hash_used;
extern int hash_bounds_adjusted;
extern int pv_node_hit;

//  MOVE EXECUTION
extern board_t* OLDSTATE[2048];
extern uint64_t HISTORY_HASHES[2048];

//  MOVE GENERATION
extern bitboard_t MASK_FILE[8];
extern bitboard_t MASK_RANK[8];
extern bitboard_t CLEAR_FILE[8];
extern bitboard_t CLEAR_RANK[8];
extern bitboard_t UNIBOARD;

extern bitboard_t ROOK_ATTACK[64][4096];
extern bitboard_t BISHOP_ATTACK[64][4096];
extern bitboard_t KNIGHT_ATTACK[64]; 
extern bitboard_t KING_ATTACK[64];


//////////////////////////////////////////////////////////////
//  HELPER FUNCTIONS
uint64_t random_uint64();
uint64_t random_uint64_fewbits();
idx_t pos_to_idx(int row, int col);
int find_1st_bit(bitboard_t bb);
int pop_1st_bit(bitboard_t *bitboard);
void initialize_attack_boards();
void initialize_helper_boards();
void initialize_oldstate_array();
void initialize_chess_engine_only_necessary();


//////////////////////////////////////////////////////////////
//  PRIORITY QUEUE FUNCTIONS
void initialize_maxpq(maxpq_t* pq);
void print_pq(maxpq_t* pq);

void sink(maxpq_t* pq, int k);
void swim(maxpq_t* pq, int k);
void swap(maxpq_t* pq, int i, int k);

void insert(maxpq_t* pq, move_t* elem);
move_t* pop_max(maxpq_t* pq);

void free_pq(maxpq_t* pq);

void heap_sort(maxpq_t* pq);

//////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS
board_t* init_board();
board_t* copy_board(board_t* board);
void recover_board(board_t* board, board_t* old_board);
void clear_board(board_t *board);
void free_board(board_t* board);
void load_by_FEN(board_t* board, char* FEN);
void update_white_black_all_boards(board_t *board);
int is_same_board(board_t *board1, board_t* board2);

/////////////////////////////////////////////////////////////
//  MOVE GENERATION
int is_capture(bitboard_t to, board_t *board);
int is_in_check(board_t *board);
int is_in_check_after_move(board_t *board);
void generate_pseudo_moves(board_t *board, maxpq_t* movelst);
void generate_moves(board_t* board, maxpq_t* movelst);

/////////////////////////////////////////////////////////////
//  MOVE & EXECUTION
move_t *generate_move(idx_t from, idx_t to, flag_t flags, uint16_t value);
move_t *copy_move(move_t *move);
void free_move(move_t *move);
int do_move(board_t* board, move_t* move);
void undo_move(board_t* board);
int is_same_move(move_t* move1, move_t* move2);

///////////////////////////////////////////////////////////////
//  SEARCH DATA
searchdata_t* init_search_data(board_t *board);
void free_search_data(searchdata_t *data);

///////////////////////////////////////////////////////////////
//  SEARCH
int alpha_beta_search(board_t *board, int depth, int alpha, int beta, searchdata_t* searchs_data);
void search(searchdata_t* search_data);

///////////////////////////////////////////////////////////////
//  PERFT 
uint64_t perft(board_t* board, int depth);
int perft_divide(board_t* board, int depth);


#endif
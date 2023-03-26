#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5


typedef uint64_t bitboard_t;
typedef uint8_t player_t;
typedef uint8_t idx_t;
typedef uint8_t flag_t;

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
} board_t;

typedef struct _move_t {
    idx_t from;
    idx_t to;
    flag_t flags;
    flag_t piece;
} move_t;

typedef struct _node_t {
    move_t* move;
    board_t* board;
    struct _node_t* next;
    struct _node_t* prev;
} node_t;


typedef struct _list_t{
    int len;
    node_t *first;
    node_t *last;
} list_t;

extern bitboard_t MASK_FILE[8];
extern bitboard_t MASK_RANK[8];
extern bitboard_t CLEAR_FILE[8];
extern bitboard_t CLEAR_RANK[8];
extern bitboard_t UNIBOARD;

extern const int ROOK_BITS[64];
extern const int BISHOP_BITS[64];
extern const uint64_t ROOK_MAGIC[64];
extern const uint64_t BISHOP_MAGIC[64];

extern bitboard_t ROOK_ATTACK[64][4096];
extern bitboard_t BISHOP_ATTACK[64][4096];
extern bitboard_t KNIGHT_ATTACK[64]; 
extern bitboard_t KING_ATTACK[64];

extern board_t* OLDSTATE[128];

//////////////////////////////////////////////////////////////
//  HELPER FUNCTIONS

extern idx_t pos_to_idx(int row, int col);
extern int find_1st_bit(bitboard_t bb);
extern int pop_1st_bit(bitboard_t *bitboard);
extern void initialize_attack_boards();
extern void initialize_helper_boards();

//////////////////////////////////////////////////////////////
//  PRINT FUNCTIONS

extern void print_board(board_t* board);
extern void print_bitboard(bitboard_t board);
extern void print_move_on_board(move_t *move);
extern void print_move(move_t* move);

//////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

extern board_t* init_board();
extern board_t* copy_board(board_t* board);
extern void recover_board(board_t* board, board_t* old_board);
extern void clear_board(board_t *board);
extern void free_board(board_t* board);
extern void load_by_FEN(board_t* board, char* FEN);
extern void update_white_black_all_boards(board_t *board);

/////////////////////////////////////////////////////////////
//  MOVE GENERATION

extern list_t* generate_pseudo_moves(board_t *board);
extern list_t* generate_moves(board_t *board);

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

extern list_t* new_list();
extern move_t* pop(list_t* head);
extern void push(list_t* head, move_t* move);

/////////////////////////////////////////////////////////////
//  MOVE

extern move_t *generate_move(idx_t from, idx_t to, flag_t flags);
extern move_t *copy_move(move_t *move);
extern void free_move(move_t *move);
extern void free_move_list(list_t *movelst);
void do_move(board_t* board, move_t* move);
void undo_move(board_t* board);

/////////////////////////////////////////////////////////////
//  MAGIC BITBOARDS

extern int transform(bitboard_t mask, uint64_t magic, int bits);
extern bitboard_t index_to_bitboard(int index, int bits, bitboard_t mask);
extern bitboard_t rook_mask(int sq);
extern bitboard_t bishop_mask(int sq);
extern bitboard_t rook_attacks(int sq, bitboard_t block);
extern bitboard_t bishop_attacks(int sq, bitboard_t block);


///////////////////////////////////////////////////////////////
// PERFT

extern uint64_t move_gen(board_t* board, int depth);
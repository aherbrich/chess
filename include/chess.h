#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

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
#define FIGURE(X) ((X)&~1)
#define OPPONENT(X) ((X == 0)?(1):(0))

#define INFINITY INT_MAX
#define NEGINFINITY (-INFINITY)

#define MAXIMIZING(X) ((X == 0)?(1):(0))

typedef uint8_t piece_t;
typedef uint8_t player_t;
typedef uint8_t flag_t;
typedef uint8_t dir_t;
typedef int8_t idx_t;
typedef uint16_t order_t;


typedef struct _board_t{
    piece_t *playingfield; 
    player_t player;
    flag_t castlerights;
    flag_t eppossible;
    piece_t epfield;
}board_t;

typedef struct _oldflags_t{
    flag_t castlerights;
    flag_t eppossible;
    piece_t epfield;
}oldflags_t;

typedef struct _move_t{
    flag_t typeofmove;

    piece_t piece_was;
    piece_t piece_is;
    piece_t piece_cap;

    idx_t start;
    idx_t end;
    idx_t startopt;
    idx_t endopt;

    flag_t newcr;
    flag_t newep;

    oldflags_t* oldflags;

    order_t orderid;
}move_t;

typedef struct _node_t{
    move_t* move;
    struct _node_t *next;
} node_t;


typedef struct _zobrist_t {
  uint64_t hashvalue[8][8][12];
} zobrist_t;

zobrist_t zobtable;

typedef struct _entry_t{
    int16_t eval;
}entry_t;


typedef struct _ttable_t{
    entry_t *entry;
}ttable_t;

/////////////////////////////////////////////////////////////
//  LIST STRUCTURE & FUNCTIONS

/* Allocate memory for list */
node_t* init_list();

/* Determine length of list */
int len(node_t* head);

/* List helpers*/
void add(node_t* head, move_t* move);
move_t* pop(node_t* head);

/* Sorts list by capture order */
node_t *sort_byorder(node_t *head);


////////////////////////////////////////////////////////////////
// MOVE FUNCTIONS

/* Allocate memory for a specific move */
move_t* create_normalmove(piece_t piece_was, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t *oldflags);
move_t* create_eppossiblemove(piece_t piece_was, idx_t start, idx_t end, flag_t newepfield, oldflags_t *oldflags);
move_t* create_promotionmove(piece_t piece_was, piece_t piece_is, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t *oldflags);
move_t* create_castlemove(idx_t startking, idx_t endking, idx_t startrook, idx_t endrook, flag_t newcr, oldflags_t *oldflags);
move_t* create_epmove(idx_t startattacker, idx_t endattacker, oldflags_t *oldflags);

/* Frees memory of move */
void free_move(move_t* move);

/* Execute move */
void playMove(board_t* board, move_t* move, player_t playerwhomademove);

/* Reverses a move/ recovers old board state */
void reverseMove(board_t* board, move_t* move, player_t playerwhomademove);

///////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

/* Allocate memory for a empty board */
board_t* init_board();

/* Free memory of board */

void free_board(board_t *board);

/* Load a game position based on FEN */
void loadByFEN(board_t *board, char *FEN);


/////////////////////////////////////////////////////////////
//  PRINT HELPERS

/* Print a piece */
char printPiece(piece_t piece);

/* Print the board */
void printBoard(board_t *board);

/* Print move*/
void printMove(move_t *move);

/* Print list of moves */
void printMoves(node_t *movelst);

/////////////////////////////////////////////////////////////
// PERFT TESTER

int MoveGen(board_t* board, int depth);

//////////////////////////////////////////////////////////////
//  USEFUL FUNCTIONS

/* Calculate idx based on a row & column */
idx_t posToIdx(int row, int col);

/* Copies flags from board */
oldflags_t* copyflags(board_t* board);

/* Copies flags from move */
oldflags_t* copyflagsfrommove(move_t* move);

//////////////////////////////////////////////////////////////
//  MOVE GENERATION

/* Move generation */
node_t* generateMoves(board_t* board);

int isLegalMove(board_t* board);


///////////////////////////////////////////////////////////////
//  EVALUATION FUNCTIONS

int countMaterial(board_t *board, player_t color);

int evalBoard(board_t* board);

///////////////////////////////////////////////////////////////
//  SEARCH 

int alphaBeta(board_t *board, int depth, int alpha, int beta, int maximizingplayer, int maxdepth, move_t** bestmove);


////////////////////////////////////////////////////////////
// ZOBRIST HASHING

void init_zobrist(zobrist_t *zobristtable);

uint64_t zobrist(zobrist_t *zobristtable, board_t *board);
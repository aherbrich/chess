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

extern char* STARTING_FEN;

typedef uint8_t piece_t;
typedef uint8_t player_t;
typedef uint8_t flag_t;
typedef uint8_t dir_t;
typedef int8_t idx_t;
typedef uint16_t order_t;

typedef struct _board_t {
    piece_t* playingfield;
    player_t player;
    flag_t castlerights;
    flag_t eppossible;
    piece_t epfield;
    uint16_t plynr;
} board_t;

typedef struct _oldflags_t {
    flag_t castlerights;
    flag_t eppossible;
    piece_t epfield;
} oldflags_t;

typedef struct _move_t {
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
} move_t;

typedef struct _node_t {
    move_t* move;
    struct _node_t* next;
} node_t;

typedef struct _bookentry_t {
    int8_t possible;
    uint64_t hash;
    move_t* move;
} bookentry_t;

extern bookentry_t book[MAXNR_LINES][MAXDEPTH_LINE];

/////////////////////////////////////////////////////////////
//  GLOBAL PERFORMANCE COUNTER

extern int nodes_searched;
extern int hash_used;
extern int hash_boundsadjusted;

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
extern move_t* create_normalmove(piece_t piece_was, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t* oldflags);
extern move_t* create_eppossiblemove(piece_t piece_was, idx_t start, idx_t end, flag_t newepfield, oldflags_t* oldflags);
extern move_t* create_promotionmove(piece_t piece_was, piece_t piece_is, piece_t piece_cap, idx_t start, idx_t end, flag_t newcr, oldflags_t* oldflags);
extern move_t* create_castlemove(idx_t startking, idx_t endking, idx_t startrook, idx_t endrook, flag_t newcr, oldflags_t* oldflags);
extern move_t* create_epmove(idx_t startattacker, idx_t endattacker, oldflags_t* oldflags);
/* Frees memory of move */
extern void free_move(move_t* move);
extern void free_movelst(node_t* movelst);
/* Deep copies move */
extern move_t* copy_move(move_t* move);
/* Execute move */
extern void playMove(board_t* board, move_t* move, player_t playerwhomademove);
/* Reverses a move/ recovers old board state */
extern void reverseMove(board_t* board, move_t* move, player_t playerwhomademove);
/* Sorts list by capture order */
extern node_t* sortMoves(node_t* head);
extern int isSameMove(move_t* move, move_t* move2);

///////////////////////////////////////////////////////////////
//  BOARD FUNCTIONS

/* Allocate memory for a empty board */
extern board_t* init_board();
/* Make deep cop of board */
extern board_t* copy_board(board_t* board);
/* Free memory of board */
extern void free_board(board_t* board);
/* Load a game position based on FEN */
extern void loadByFEN(board_t* board, char* FEN);

/////////////////////////////////////////////////////////////
//  PRINT HELPERS

/* Print a piece */
extern char printPiece(piece_t piece);
/* Print the board */
extern void printBoard(board_t* board);
/* Print move*/
extern void printMove(move_t* move);
/* Print list of moves */
extern void printMoves(node_t* movelst);
extern void printLine(board_t* board, int depth);

/////////////////////////////////////////////////////////////
// PERFT TESTER

/* Perft move generation and validation */
extern int MoveGen(board_t* board, int depth);

//////////////////////////////////////////////////////////////
//  USEFUL FUNCTIONS

/* Calculate idx based on a row & column */
extern idx_t posToIdx(int row, int col);
/* Copies flags from board */
extern oldflags_t* copyflags(board_t* board);
/* Copies flags from move */
extern oldflags_t* copyflagsfrommove(move_t* move);
/* Max of function */
extern int maxof(int x, int y);
/* Min of function */
extern int minof(int x, int y);

//////////////////////////////////////////////////////////////
//  MOVE GENERATION

/* Move generation */
extern node_t* generateMoves(board_t* board);
/* Generates all captures */
extern node_t* generateCaptures(board_t* board);
/* Checks if a move is legal (king not in check) */
extern int isLegalMove(board_t* board);
extern int PVMoveIsPossible(node_t* movelst, move_t* ttmove);

///////////////////////////////////////////////////////////////
//  EVALUATION FUNCTIONS

/* Counts the material and its value of a specific color */
extern int countMaterial(board_t* board, player_t color);
/* Simple evaluation fucntion for negamax */
extern int evalBoardMax(board_t* board);
/* Simple evaluation function of an ended game for negamax*/
extern int evalEndOfGameMax(board_t* board, int depth);

///////////////////////////////////////////////////////////////
//  SEARCH

/* The alpha beta search */
extern int alphaBetaWithTT(board_t* board, uint8_t depth, int alpha, int beta, clock_t start, double timeleft);
extern move_t* iterativeSearch(board_t* board, int8_t maxdepth, double maxtime);

//////////////////////////////////////////////////////////////
// BOOK OF OPENINGS

extern void init_book();
extern int book_possible(board_t* board);
extern move_t* getRandomBook(board_t* board);

#endif
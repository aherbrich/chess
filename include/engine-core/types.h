#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdlib.h>
#ifdef __linux__
#include <stdint.h>
#endif

#define WHITE 1
#define BLACK 0
#define SWITCHSIDES(X) (X^WHITE)

typedef enum _file_t {
    A, B, C, D, E, F, G, H
} file_t;

typedef enum _rank_t {
    RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8
} rank_t;

typedef enum _ordervalue_t {
    EMPTY, PAWN_ID, KNIGHT_ID, BISHOP_ID, ROOK_ID, QUEEN_ID, KING_ID
} ordervalue_t;

typedef enum _piecetype_t {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
} piecetype_t;

typedef enum _castleflag_t {
    LONGSIDEW = 1, SHORTSIDEW = 2, LONGSIDEB = 4, SHORTSIDEB = 8
} castleflag_t;

typedef enum _moveflags_t {
    QUIET = 0, DOUBLEP, KCASTLE, QCASTLE, CAPTURE, EPCAPTURE,
    KPROM=8, BPROM, RPROM, QPROM, KCPROM, BCPROM, RCPROM, QCPROM
} moveflags_t;

typedef enum _square_t {
	a1=0, ll, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
	NO_SQUARE
} square_t;

#define NR_PIECES 15
typedef enum _piece_t {
	B_PAWN=0, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
	W_PAWN=8, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    NO_PIECE
} piece_t;

typedef enum _dir_t {
    NORTH = 8, NORTH_EAST = 9, EAST = 1, SOUTH_EAST = -7,
	SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7,
	NORTH_NORTH = 16, SOUTH_SOUTH = -16
} dir_t;

/* useful renames */
typedef uint64_t bitboard_t;
typedef uint8_t player_t;
typedef uint8_t idx_t;
typedef uint8_t flag_t;

/* struct holding information (lost in do_move, but)
   needed for reconstruction of board in undo_move */
typedef struct _undoinfo_t {
    flag_t castlerights;
    piece_t captured;
    square_t epsq;

    uint16_t full_move_counter;
    uint8_t fifty_move_counter;

    uint64_t hash;
} undoinfo_t;

/* structure representing a chessboard */
#define MAXPLIES 1024
typedef struct _board_t {
    piece_t playingfield[64];
    bitboard_t piece_bb[NR_PIECES];
    bitboard_t checkers;
    bitboard_t pinned;
    bitboard_t attackmap;
    
    player_t player;

    undoinfo_t history[MAXPLIES];

    uint16_t ply_no;

    uint64_t hash;
} board_t;

/* structure representing a move */
typedef struct _move_t {
    uint16_t value;
    idx_t from;
    idx_t to;
    flag_t flags;
} move_t;

#endif

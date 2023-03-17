#include <stdint.h>

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

    player_t player;
    flag_t castle_rights;
    flag_t ep_possible;
    idx_t ep_field;
    uint16_t ply_no;
} board_t;

//////////////////////////////////////////////////////
/// HELPER FUNCTIONS

/* determines index from row and column */
idx_t pos_to_idx(int row, int col);
/* Returns index of least significant bit */
int find_1st_bit(bitboard_t bb);
/* Sets the LS1B (least significant 1 bit) to 0 and returns it index */
int pop_1st_bit(bitboard_t *bitboard);

/////////////////////////////////////////////////////
/// PRINT FUNCTIONS

/* prints the board */
void print_board(board_t* board);

//////////////////////////////////////////////////////
/// BOARD FUNCTIONS

/* Allocate memory for a empty board */
board_t* init_board();
/* Clear the board */
void clear_board(board_t *board);
/* Copy given board */
board_t* copy_board(board_t* board);
/* Free memory of board */
void free_board(board_t* board);
/* Load a game position based on FEN */
void load_by_FEN(board_t* board, char* FEN);

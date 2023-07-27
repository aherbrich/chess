#include "../include/chess.h"
#include "../include/magic.h"
#include "string.h"
#include "../include/prettyprint.h"

bitboard_t MASK_FILE[8] = {
	0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
	0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080
};

bitboard_t CLEAR_FILE[8] = {
    18374403900871474942ULL, 18302063728033398269ULL, 18157383382357244923ULL, 17868022691004938231ULL,
    17289301308300324847ULL, 16131858542891098079ULL, 13816973012072644543ULL,  9187201950435737471ULL
};

bitboard_t MASK_RANK[8] = {
	0xff, 0xff00, 0xff0000, 0xff000000,
	0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

bitboard_t CLEAR_RANK[8] = {
    18446744073709551360ULL, 18446744073709486335ULL, 18446744073692839935ULL, 18446744069431361535ULL, 
    18446742978492891135ULL, 18446463698244468735ULL, 18374967954648334335ULL,    72057594037927935ULL
};

bitboard_t UNIBOARD = 18446744073709551615ULL;

bitboard_t MASK_DIAGONAL[15] = {
	0x80, 0x8040, 0x804020,
	0x80402010, 0x8040201008, 0x804020100804,
	0x80402010080402, 0x8040201008040201, 0x4020100804020100,
	0x2010080402010000, 0x1008040201000000, 0x804020100000000,
	0x402010000000000, 0x201000000000000, 0x100000000000000,
};

bitboard_t MASK_ANTI_DIAGONAL[15] = {
	0x1, 0x102, 0x10204,
	0x1020408, 0x102040810, 0x10204081020,
	0x1020408102040, 0x102040810204080, 0x204081020408000,
	0x408102040800000, 0x810204080000000, 0x1020408000000000,
	0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

bitboard_t ROOK_ATTACK[64][4096];
bitboard_t BISHOP_ATTACK[64][4096];
bitboard_t KNIGHT_ATTACK[64];
bitboard_t KING_ATTACK[64];
bitboard_t ROOK_ATTACK_MASK[64];
bitboard_t BISHOP_ATTACK_MASK[64];

bitboard_t PAWN_ATTACK[2][64] = {{
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
	0x2, 0x5, 0xa, 0x14,
	0x28, 0x50, 0xa0, 0x40,
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
}, {
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
	0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
	0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
}
};

const bitboard_t SQUARE_BB[65] = {
	0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x100000000, 0x200000000, 0x400000000, 0x800000000,
	0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
	0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
	0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
	0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
	0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
	0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
	0x0
};

bitboard_t SQUARES_BETWEEN_BB[64][64];
bitboard_t LINE[64][64];

/* Checks if piece would capture on a given TO-field and returns the piece type
 */
int is_capture(bitboard_t to, board_t *board) {
    bitboard_t to_mask = (1ULL << to);
    if (board->player == WHITE) {
        if (board->black & to_mask) {
            if (board->piece_bb[B_PAWN] & to_mask) return PAWN;
            if (board->piece_bb[B_KNIGHT] & to_mask) return KNIGHT;
            if (board->piece_bb[B_BISHOP] & to_mask) return BISHOP;
            if (board->piece_bb[B_ROOK] & to_mask) return ROOK;
            if (board->piece_bb[B_QUEEN] & to_mask) return QUEEN;
            if (board->piece_bb[B_KING] & to_mask) return KING;
        }
        return EMPTY;
    } else {
        if (board->white & to_mask) {
            if (board->piece_bb[W_PAWN] & to_mask) return PAWN;
            if (board->piece_bb[W_KNIGHT] & to_mask) return KNIGHT;
            if (board->piece_bb[W_BISHOP] & to_mask) return BISHOP;
            if (board->piece_bb[W_ROOK] & to_mask) return ROOK;
            if (board->piece_bb[W_QUEEN] & to_mask) return QUEEN;
            if (board->piece_bb[W_KING] & to_mask) return KING;
        }
        return EMPTY;
    }
}

/* Checks if a player is in check WHILE CURRENTLY AT TURN */
/* WARNING: (Generally) Call BEFORE making a move  */
int is_in_check(board_t *board) {
    if (board->player == WHITE) {
        if (!board->piece_bb[W_KING]) return TRUE;
        int king_sq = find_1st_bit(board->piece_bb[W_KING]);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */
        attackers = KNIGHT_ATTACK[king_sq] & board->piece_bb[B_KNIGHT];
        if (attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = ROOK_ATTACK_MASK[king_sq] & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->piece_bb[B_ROOK];
        if (attackers) return TRUE;

        blockers = BISHOP_ATTACK_MASK[king_sq] & board->all;
        int k =
            transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->piece_bb[B_BISHOP];
        if (attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) &
                    board->piece_bb[B_QUEEN];
        if (attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->piece_bb[W_KING] & CLEAR_FILE[A]) << 7) |
                     ((board->piece_bb[W_KING] & CLEAR_FILE[H]) << 9)) &
                    board->piece_bb[B_PAWN];
        if (attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->piece_bb[B_KING];
        if (attackers) return TRUE;

        return FALSE;
    } else {
        if (!board->piece_bb[B_KING]) return TRUE;
        int king_sq = find_1st_bit(board->piece_bb[B_KING]);

        bitboard_t attackers;
        bitboard_t blockers;

        /* check by knights */
        attackers = KNIGHT_ATTACK[king_sq] & board->piece_bb[W_KNIGHT];
        if (attackers) return TRUE;

        /* check by bishops, rooks, and queens */
        blockers = ROOK_ATTACK_MASK[king_sq] & board->all;
        int j = transform(blockers, ROOK_MAGIC[king_sq], ROOK_BITS[king_sq]);
        attackers = ROOK_ATTACK[king_sq][j] & board->piece_bb[W_ROOK];
        if (attackers) return TRUE;

        blockers = BISHOP_ATTACK_MASK[king_sq] & board->all;
        int k =
            transform(blockers, BISHOP_MAGIC[king_sq], BISHOP_BITS[king_sq]);
        attackers = BISHOP_ATTACK[king_sq][k] & board->piece_bb[W_BISHOP];
        if (attackers) return TRUE;

        attackers = (BISHOP_ATTACK[king_sq][k] | ROOK_ATTACK[king_sq][j]) &
                    board->piece_bb[W_QUEEN];
        if (attackers) return TRUE;

        /* check by pawns */
        attackers = (((board->piece_bb[B_KING] & CLEAR_FILE[H]) >> 7) |
                     ((board->piece_bb[B_KING] & CLEAR_FILE[A]) >> 9)) &
                    board->piece_bb[W_PAWN];
        if (attackers) return TRUE;

        /* 'check by king' */
        attackers = KING_ATTACK[king_sq] & board->piece_bb[W_KING];
        if (attackers) return TRUE;

        return FALSE;
    }
}

/* Generates all legal moves for player at turn */
void generate_moves(board_t *board, maxpq_t *movelst) {
    generate_legals(board, movelst);
}

int sparse_pop_count(bitboard_t x) {
	int count = 0;
	while (x) {
		count++;
		x &= x - 1;
	}
	return count;
}

bitboard_t shift(bitboard_t bb, int direction){
    switch(direction){
        case NORTH: return bb << 8;
        case SOUTH: return bb >> 8;
        case NORTH_NORTH: return bb << 16;
        case SOUTH_SOUTH: return bb >> 16;
        case EAST: return ((bb & CLEAR_FILE[H]) << 1);
        case WEST: return ((bb & CLEAR_FILE[A]) >> 1);
        case NORTH_EAST: return ((bb & CLEAR_FILE[H]) << 9);
        case NORTH_WEST: return ((bb & CLEAR_FILE[A]) << 7);
        case SOUTH_EAST: return ((bb & CLEAR_FILE[H]) >> 7);
        case SOUTH_WEST: return ((bb & CLEAR_FILE[A]) >> 9);
        default: 
            fprintf(stderr, "This should not happen!"); 
            exit(1);
    }
}

bitboard_t diagonal_sliders(board_t* board, player_t player) {
	return (player == WHITE) ? board->piece_bb[W_BISHOP] | board->piece_bb[W_QUEEN] :
		board->piece_bb[B_BISHOP] | board->piece_bb[B_QUEEN];
}

bitboard_t orthogonal_sliders(board_t* board, player_t player) {
	return (player == WHITE) ? board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] :
		board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN];
}

bitboard_t attack_pawn_multiple(bitboard_t p, player_t player) {
	return player == WHITE ? shift(p, NORTH_WEST) | shift(p, NORTH_EAST) :
		shift(p, SOUTH_WEST) | shift(p, SOUTH_EAST);
}

bitboard_t attack_pawn_single(square_t sq, player_t player){
    return PAWN_ATTACK[player][sq];
}

bitboard_t attack_bishop(square_t sq, bitboard_t occ){
    /* use magic bitboards to get attack squares */
    int j = transform(occ & BISHOP_ATTACK_MASK[sq], BISHOP_MAGIC[sq], BISHOP_BITS[sq]);
    return BISHOP_ATTACK[sq][j];
}

bitboard_t attack_rook(square_t sq, bitboard_t occ){
    /* use magic bitboards to get attack squares */
    int j = transform(occ & ROOK_ATTACK_MASK[sq], ROOK_MAGIC[sq], ROOK_BITS[sq]);
    return ROOK_ATTACK[sq][j];
}

bitboard_t attackers_from(board_t* board, square_t sq, bitboard_t occ, player_t player) {
	return (player == WHITE) ? 
        ((attack_pawn_single(sq, BLACK) & board->piece_bb[W_PAWN]) | 
        (KNIGHT_ATTACK[sq] & board->piece_bb[W_KNIGHT]) |
        (attack_bishop(sq, occ) & (board->piece_bb[W_BISHOP] | board->piece_bb[W_QUEEN])) |
        (attack_rook(sq, occ) & (board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN]))) :
        ((attack_pawn_single(sq, WHITE) & board->piece_bb[B_PAWN]) | 
        (KNIGHT_ATTACK[sq] & board->piece_bb[B_KNIGHT]) |
        (attack_bishop(sq, occ) & (board->piece_bb[B_BISHOP] | board->piece_bb[B_QUEEN])) |
        (attack_rook(sq, occ) & (board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN])));
}

dir_t relative_dir(player_t player, dir_t d) {
	return (player == WHITE) ? (dir_t) d : (dir_t) -d;
}

dir_t relative_rank(player_t player, dir_t r) {
	return (player == WHITE) ? (dir_t) r : (dir_t) (RANK8 - r);
}

int oo_allowed(player_t player, flag_t cr){
    return (player == WHITE) ? (cr & SHORTSIDEW) : (cr & SHORTSIDEB);
}

int ooo_allowed(player_t player, flag_t cr){
    return (player == WHITE) ? (cr & LONGSIDEW) : (cr & LONGSIDEB);
}

bitboard_t oo_blockers_mask(player_t player) { 
	return (player == WHITE) ? 96ULL : 6917529027641081856ULL; 
}

bitboard_t ooo_blockers_mask(player_t player) { 
	return (player == WHITE) ? 14ULL : 1008806316530991104ULL; 
}

bitboard_t ignore_ooo_danger_bfile(player_t player) { return player == WHITE ? 0x2 : 0x200000000000000; }

square_t make_piece(player_t player, int pc){
    return (player == WHITE) ? (square_t) (0b1000 & pc) : (square_t) (pc);
}

void make_moves_quiet(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), QUIET, 0));
}

void make_moves_doubleep(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), DOUBLEP, 0));
}

void make_moves_capture(maxpq_t *movelst, board_t* board, square_t from, bitboard_t targets){
    while (targets) {
        int p = pop_1st_bit(&targets);
        insert(movelst, generate_move(from, p, CAPTURE, board->piece_bb[p] * 100 + (KING - (board->piece_bb[from] & 0b111))));
    }
}

void make_moves_epcapture(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets) insert(movelst, generate_move(from, pop_1st_bit(&targets), EPCAPTURE, 0));
}

void make_moves_promcaptures(maxpq_t *movelst, square_t from, bitboard_t targets){
    while (targets){
        int idx = pop_1st_bit(&targets);
        insert(movelst, generate_move(from, idx, KCPROM, 2500));
        insert(movelst, generate_move(from, idx, BCPROM, 2600));
        insert(movelst, generate_move(from, idx, RCPROM, 2700));
        insert(movelst, generate_move(from, idx, QCPROM, 2800));
    }
}

void generate_legals(board_t* board, maxpq_t *movelst){
    player_t us = board->player;
    player_t them = SWITCHSIDES(us);

	bitboard_t us_bb = (us == WHITE) ? 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]) : 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]);
	bitboard_t them_bb = (us == WHITE) ? 
                (board->piece_bb[B_PAWN] | board->piece_bb[B_KNIGHT] | board->piece_bb[B_BISHOP] | 
                board->piece_bb[B_ROOK] | board->piece_bb[B_QUEEN] | board->piece_bb[B_KING]) : 
                (board->piece_bb[W_PAWN] | board->piece_bb[W_KNIGHT] | board->piece_bb[W_BISHOP] | 
                board->piece_bb[W_ROOK] | board->piece_bb[W_QUEEN] | board->piece_bb[W_KING]);
	bitboard_t all = us_bb | them_bb;

	square_t our_king_sq = (us == WHITE) ? find_1st_bit(board->piece_bb[W_KING]) : find_1st_bit(board->piece_bb[B_KING]);
	square_t their_king_sq = (us == WHITE) ? find_1st_bit(board->piece_bb[B_KING]) : find_1st_bit(board->piece_bb[W_KING]);

	bitboard_t our_diag_sliders_bb = diagonal_sliders(board, us);
	bitboard_t their_diag_sliders_bb = diagonal_sliders(board, them);
	bitboard_t our_orth_sliders_bb = orthogonal_sliders(board, us);
	bitboard_t their_orth_sliders_bb = orthogonal_sliders(board, them);

    bitboard_t our_pawns_bb = (us == WHITE) ? board->piece_bb[W_PAWN] : board->piece_bb[B_PAWN];
    bitboard_t our_knights_bb = (us == WHITE) ? board->piece_bb[W_KNIGHT] : board->piece_bb[B_KNIGHT];

    bitboard_t their_pawns_bb = (them == WHITE) ? board->piece_bb[W_PAWN] : board->piece_bb[B_PAWN];
    bitboard_t their_knights_bb = (them == WHITE) ? board->piece_bb[W_KNIGHT] : board->piece_bb[B_KNIGHT];

    //General purpose bitboards for attacks, masks, etc.
	bitboard_t b1, b2, b3;
	
	//Squares that our king cannot move to
	bitboard_t danger = 0ULL;

    // For each enemy piece, add all of its attacks to the danger bitboard
    // attacks by enemy pawns and enemy kings
	danger |= attack_pawn_multiple(their_pawns_bb, them) | KING_ATTACK[their_king_sq];

    // attacks by enemy knights
    b1 = their_knights_bb; 
	while (b1) danger |=  KNIGHT_ATTACK[pop_1st_bit(&b1)];

    // diagonal attacks by bishop and queen
    b1 = their_diag_sliders_bb;
	// all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	// by enemy bishops and queens
	while (b1) danger |= attack_bishop(pop_1st_bit(&b1), all ^ SQUARE_BB[our_king_sq]);

    // orthogonal attacks by rook and queen
    b1 = their_orth_sliders_bb;
	//all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	//by enemy rooks and queens
	while (b1) danger |= attack_rook(pop_1st_bit(&b1), all ^ SQUARE_BB[our_king_sq]);

    //The king can move to all of its surrounding squares, except ones that are attacked, and
	//ones that have our own pieces on them
	b1 = KING_ATTACK[our_king_sq] & ~(us_bb | danger);
    make_moves_quiet(movelst, our_king_sq, b1 & ~them_bb);
    make_moves_capture(movelst, board, our_king_sq, b1 & them_bb);

    //The capture mask filters destination squares to those that contain an enemy piece that is checking the 
	//king and must be captured
	bitboard_t capture_mask;
	
	//The quiet mask filter destination squares to those where pieces must be moved to block an incoming attack 
	//to the king
    bitboard_t quiet_mask;
	
	//A general purpose square for storing destinations, etc.
	square_t s;

    //Checkers of each piece type are identified by:
	//1. Projecting attacks FROM the king square
	//2. Intersecting this bitboard with the enemy bitboard of that piece type
	bitboard_t checkers = (KNIGHT_ATTACK[our_king_sq] & their_knights_bb) | (attack_pawn_single(our_king_sq, us) & their_pawns_bb);

    //Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners 
	//and checkers are represented by the bitboard <candidates>
	bitboard_t candidates = (attack_rook(our_king_sq, them_bb) & their_orth_sliders_bb) | (attack_bishop(our_king_sq, them_bb) & their_diag_sliders_bb);

    bitboard_t pinned = 0;
	while (candidates) {
		s = pop_1st_bit(&candidates);
		b1 = SQUARES_BETWEEN_BB[our_king_sq][s] & us_bb;
		
		//Do the squares in between the enemy slider and our king contain any of our pieces?
		//If not, add the slider to the checker bitboard
		if (b1 == 0) checkers ^= SQUARE_BB[s];
		//If there is only one of our pieces between them, add our piece to the pinned bitboard 
		else if ((b1 & b1 - 1) == 0) pinned ^= b1;
	}

    //This makes it easier to mask pieces
	bitboard_t not_pinned = ~pinned;

    switch (sparse_pop_count(checkers)) {
	case 2:
		//If there is a double check, the only legal moves are king moves out of check
		return;
	case 1: {
		//It's a single check!

		square_t checker_square = find_1st_bit(checkers);

		switch (board->playingfield[checker_square] &0b111) {
		case BW_PAWN:
			//If the checker is a pawn, we must check for e.p. moves that can capture it
			//This evaluates to true if the checking piece is the one which just double pushed
			if (checkers == shift(SQUARE_BB[board->history[board->ply_no].epsq], relative_dir(us, SOUTH))) {
				//b1 contains our pawns that can capture the checker e.p.
				b1 = attack_pawn_single(board->history[board->ply_no].epsq, them) & our_pawns_bb & not_pinned;
                
                while (b1) insert(movelst, generate_move(pop_1st_bit(&b1), board->history[board->ply_no].epsq, EPCAPTURE, 0));
			}
			//FALL THROUGH INTENTIONAL
		case BW_KNIGHT:
			//If the checker is either a pawn or a knight, the only legal moves are to capture
			//the checker. Only non-pinned pieces can capture it
			b1 = attackers_from(board, checker_square, all, us) & not_pinned;
			while (b1) {
                int p = pop_1st_bit(&b1);
                insert(movelst, generate_move(p , checker_square, CAPTURE, board->piece_bb[checker_square]*100 + (KING - (board->piece_bb[p] & 0b111))));
            }
			return;
		default:
			//We must capture the checking piece
			capture_mask = checkers;
			
			//...or we can block it since it is guaranteed to be a slider
			quiet_mask = SQUARES_BETWEEN_BB[our_king_sq][checker_square];
			break;
		}

		break;
	}

    default: {
        //We can capture any enemy piece
		capture_mask = them_bb;
		
		//...and we can play a quiet move to any square which is not occupied
		quiet_mask = ~all;

		if (board->history[board->ply_no].epsq != NO_SQUARE) {
			//b1 contains our pawns that can perform an e.p. capture
			b2 = attack_pawn_single(board->history[board->ply_no].epsq, them) & our_pawns_bb;
			b1 = b2 & not_pinned;
			while (b1) {
				s = pop_1st_bit(&b1);
				
				//This piece of evil bit-fiddling magic prevents the infamous 'pseudo-pinned' e.p. case,
				//where the pawn is not directly pinned, but on moving the pawn and capturing the enemy pawn
				//e.p., a rook or queen attack to the king is revealed
				
				/*
				.nbqkbnr
				ppp.pppp
				........
				r..pP..K
				........
				........
				PPPP.PPP
				RNBQ.BNR
				
				Here, if white plays exd5 e.p., the black rook on a5 attacks the white king on h5 
				*/
                if (((sliding_attacks(our_king_sq, 
                                        all ^ SQUARE_BB[s] ^ shift(SQUARE_BB[board->history[board->ply_no].epsq], relative_dir(us, SOUTH)), 
                                        MASK_RANK[rank_of(our_king_sq)]) 
                    & their_orth_sliders_bb) == 0)){
                        insert(movelst, generate_move(s, board->history[board->ply_no].epsq, EPCAPTURE, 0));
                }
			}
			
			//Pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king 
			b1 = b2 & pinned & LINE[board->history[board->ply_no].epsq][our_king_sq];
			if (b1) {
                insert(movelst, generate_move(find_1st_bit(b1), board->history[board->ply_no].epsq, EPCAPTURE, 0));
			}
		}

		//Only add castling if:
		//1. The king and the rook have both not moved
		//2. No piece is attacking between the the rook and the king
		//3. The king is not in check
        if(!((all | danger) & oo_blockers_mask(us)) && (oo_allowed(us, board->history[board->ply_no].castlerights))){
            if(us == WHITE){
                insert(movelst, generate_move(e1, g1, KCASTLE, 0));
            } else{
                insert(movelst, generate_move(e8, g8, KCASTLE, 0));
            }
        }
        if(!((all | (danger & ~ignore_ooo_danger_bfile(us))) & ooo_blockers_mask(us)) && (ooo_allowed(us, board->history[board->ply_no].castlerights))){
            if(us == WHITE){
                insert(movelst, generate_move(e1, c1, QCASTLE, 0));
            } else{
                insert(movelst, generate_move(e8, c8, QCASTLE, 0));
            }
        }

		//For each pinned rook, bishop or queen...
		b1 = ~(not_pinned | our_knights_bb | our_pawns_bb);
		while (b1) {
			s = pop_1st_bit(&b1);
			
			//...only include attacks that are aligned with our king, since pinned pieces
			//are constrained to move in this direction only
            switch(board->playingfield[s] & 0b111){
                case BW_BISHOP:
                    b2 = attack_bishop(s, all) & LINE[our_king_sq][s];
                    break;
                case BW_ROOK:
                    b2 = attack_rook(s, all) & LINE[our_king_sq][s];
                    break;
                case BW_QUEEN:
                    b2 = (attack_bishop(s, all) | attack_rook(s, all)) & LINE[our_king_sq][s];
            }
            make_moves_quiet(movelst, s, b2 & quiet_mask);
            make_moves_capture(movelst, board,  s, b2 & capture_mask);
		}

		//For each pinned pawn...
		b1 = ~not_pinned & our_pawns_bb;
		while (b1) {
			s = pop_1st_bit(&b1);

			if (rank_of(s) == relative_rank(us, RANK7)) {
				//Quiet promotions are impossible since the square in front of the pawn will
				//either be occupied by the king or the pinner, or doing so would leave our king
				//in check
				b2 = attack_pawn_single(s, us) & capture_mask & LINE[our_king_sq][s];
                make_moves_promcaptures(movelst, s, b2);
			}
			else {
                b2 = attack_pawn_single(s, us) & capture_mask & LINE[our_king_sq][s];
                make_moves_capture(movelst, board, s, b2);
				
				//Single pawn pushes
				b2 = shift(SQUARE_BB[s], relative_dir(us, NORTH)) & ~all & LINE[our_king_sq][s];
				//Double pawn pushes (only pawns on rank 3/6 are eligible)
				b3 = shift(b2 & MASK_RANK[relative_rank(us, RANK3)], 
                                relative_dir(us, NORTH)) & ~all & LINE[our_king_sq][s];
                make_moves_quiet(movelst, s, b2);
                make_moves_doubleep(movelst, s, b3);
			}
		}
		
		//Pinned knights cannot move anywhere, so we're done with pinned pieces!

		break;
    }
    
    }

    //Non-pinned knight moves
	b1 = our_knights_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = KNIGHT_ATTACK[s];
        make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}

    //Non-pinned bishops and queens
	b1 = our_diag_sliders_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = attack_bishop(s, all);
        make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}

    //Non-pinned rooks and queens
	b1 = our_orth_sliders_bb & not_pinned;
	while (b1) {
		s = pop_1st_bit(&b1);
		b2 = attack_rook(s, all);
		make_moves_quiet(movelst, s, b2 & quiet_mask);
        make_moves_capture(movelst, board, s, b2 & capture_mask);
	}


    //b1 contains non-pinned pawns which are not on the last rank
	b1 = our_pawns_bb & not_pinned & ~MASK_RANK[relative_rank(us, RANK7)];
    //Single pawn pushes
	b2 = shift(b1, relative_dir(us, NORTH)) & ~all;
	
	//Double pawn pushes (only pawns on rank 3/6 are eligible)
	b3 = shift(b2 & MASK_RANK[relative_rank(us, RANK3)], relative_dir(us, NORTH)) & quiet_mask;

    //We & this with the quiet mask only later, as a non-check-blocking single push does NOT mean that the 
	//corresponding double push is not blocking check either.
	b2 &= quiet_mask;

    while (b2) {
		s = pop_1st_bit(&b2);
        insert(movelst, generate_move(s-relative_dir(us, NORTH), s, QUIET, 0));
	}

	while (b3) {
		s = pop_1st_bit(&b3);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_NORTH), s, DOUBLEP, 0));
	}

    //Pawn captures
	b2 = shift(b1, relative_dir(us, NORTH_WEST)) & capture_mask;
	b3 = shift(b1, relative_dir(us, NORTH_EAST)) & capture_mask;

	while (b2) {
		s = pop_1st_bit(&b2);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, CAPTURE, board->piece_bb[s]*100 + (KING - PAWN)));
	}

	while (b3) {
		s = pop_1st_bit(&b3);
        insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, CAPTURE, board->piece_bb[s]*100 + (KING - PAWN)));
	}

    //b1 now contains non-pinned pawns which ARE on the last rank (about to promote)
	b1 = our_pawns_bb & not_pinned & MASK_RANK[relative_rank(us, RANK7)];
	if (b1) {
		//Quiet promotions
		b2 = shift(b1, relative_dir(us, NORTH)) & quiet_mask;
		while (b2) {
			s = pop_1st_bit(&b2);
			//One move is added for each promotion piece
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, KPROM, 2100));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, BPROM, 2200));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, RPROM, 2300));
            insert(movelst, generate_move(s-relative_dir(us, NORTH), s, QPROM, 2400));
		}

		//Promotion captures
		b2 = shift(b1, relative_dir(us, NORTH_WEST)) & capture_mask;
		b3 = shift(b1, relative_dir(us, NORTH_EAST)) & capture_mask;

		while (b2) {
			s = pop_1st_bit(&b2);
			//One move is added for each promotion piece
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, KCPROM, 2500));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, BCPROM, 2600));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, RCPROM, 2700));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_WEST), s, QCPROM, 2800));
		}

		while (b3) {
			s = pop_1st_bit(&b3);
			//One move is added for each promotion piece
			insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, KCPROM, 2500));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, BCPROM, 2600));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, RCPROM, 2700));
            insert(movelst, generate_move(s-relative_dir(us, NORTH_EAST), s, QCPROM, 2800));
		}
	}

}
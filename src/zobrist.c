#include "../include/chess.h"

zobrist_t zobtable;
htentry_t *httable;

idx_t indexing(piece_t piece){
    switch(piece){
        case PAWN|WHITE: return 0;
        case KNIGHT|WHITE: return 1;
        case BISHOP|WHITE: return 2;
        case ROOK|WHITE: return 3;
        case QUEEN|WHITE: return 4;
        case KING|WHITE: return 5;

        case PAWN|BLACK: return 6;
        case KNIGHT|BLACK: return 7;
        case BISHOP|BLACK: return 8;
        case ROOK|BLACK: return 9;
        case QUEEN|BLACK: return 10;
        case KING|BLACK: return 11;
        default: return -1;
    }
}

uint64_t get64rand() {
    return
    (((uint64_t) rand() <<  0) & 0x000000000000FFFFull) |
    (((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) |
    (((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
    (((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
}

void init_zobrist() {
	for (int x = 0 ; x < 8 ; x++) {
		for (int y = 0 ; y < 8 ; y++) {
            for(int piece = 0; piece < 12; piece++){
                zobtable.hashvalue[x][y][piece]  = get64rand();
            }    
		}
    }
    for(int i = 0; i < 13; i++){
        zobtable.hashflags[i] = get64rand();
    }
}

uint64_t zobrist(board_t *board){
    int hash = 0;

    for(int x = 0; x<8; x++){
        for(int y = 0; y < 8; y++){
            piece_t piece = board->playingfield[posToIdx(x, y)];
            if(piece != EMPTY){
                idx_t pieceidx = indexing(piece);
                hash ^= zobtable.hashvalue[x][y][pieceidx];
            }
        }
    }

    if(board->eppossible){
        hash ^= zobtable.hashflags[board->epfield%8];
    }

    if((board->castlerights&LONGSIDEW) != 0){
        hash ^= zobtable.hashflags[8];
    }

    if((board->castlerights&SHORTSIDEW) != 0){
        hash ^= zobtable.hashflags[9];
    }

    if((board->castlerights&LONGSIDEB) != 0){
        hash ^= zobtable.hashflags[10];
    }

    if((board->castlerights&SHORTSIDEB) != 0){
        hash ^= zobtable.hashflags[11];
    }

    if(board->player == BLACK){
        hash ^= zobtable.hashflags[12];
    }

    return hash;
}

void init_hashtable() {
	httable = (htentry_t*) malloc(sizeof(htentry_t)*HTSIZE);
    for(int i = 0; i < HTSIZE; i++){
        httable[i].flags = 0;
        httable[i].eval = 0;
        
        httable[i].depth = -1;
        httable[i].alpha = 0;
        httable[i].beta = 0;
        httable[i].bestmove = NULL;
        httable[i].hash = 0;
    }
}
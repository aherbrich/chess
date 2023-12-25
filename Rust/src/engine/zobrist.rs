use const_random::const_random;

pub const ZOBRIST_TABLE: ZobristTable = ZobristTable::new();

pub struct ZobristTable {
    pub pieces: [[u64; 64]; 12],
    pub flags: [u64; 26]        // 26 because of 8 en passant files, 16 for castling rights and 2 for side to move
}

impl ZobristTable {
    const fn new() -> ZobristTable {
        let mut zobrist_table = ZobristTable {
            pieces: [[0; 64]; 12],
            flags: [0; 26]
        };

        let mut i = 0;
        while i < 12 {
            let mut j = 0;
            while j < 64 {
                zobrist_table.pieces[i][j] = const_random!(u64);
                j += 1;
            }
            i += 1;
        }

        let mut k = 0;
        while k < 26 {
            zobrist_table.flags[k] = const_random!(u64);
            k += 1;
        }

        zobrist_table
    }
}
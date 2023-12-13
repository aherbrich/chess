use crate::helpers::{rank_of, file_of, diagonal_of, anti_diagonal_of, attacks_on_line, transform, first_bit};
use crate::types::Player;

// helper bitboards 
const MASK_FILE : [u64; 8] = initialize_mask_file_table();
pub const MASK_RANK : [u64; 8] = initialize_mask_rank_table();
pub const CLEAR_FILE : [u64; 8] = initialize_clear_file_table();
const CLEAR_RANK : [u64; 8] = initialize_clear_rank_table();
const MASK_ANTI_DIAG : [u64; 15] = initialize_mask_anti_diag_table();
const MASK_DIAG : [u64; 15] = initialize_mask_diag_table();
pub const SQUARE_BB : [u64; 65] = initialize_square_bb_table();
const UNIBOARD : u64 = 0xFFFFFFFFFFFFFFFF;
pub const SQUARES_BETWEEN_BB : [[u64; 64]; 64] = initialize_squares_between_bb_table();
pub const LINE_SPANNED_BB : [[u64; 64]; 64] = initialize_line_spanned_bb_table();

// magic bitboard helper arrays for sliding pieces
pub const ROOK_BITS : [u64; 64] = initialize_rook_bits_table();
pub const BISHOP_BITS : [u64; 64] = initialize_bishop_bits_table();
pub const ROOK_MAGIC : [u64; 64] = initialize_rook_magic_table();
pub const BISHOP_MAGIC : [u64; 64] = initialize_bishop_magic_table();
pub const ROOK_MASK : [u64; 64] = initialize_rook_mask_table();
pub const BISHOP_MASK : [u64; 64] = initialize_bishop_mask_table();

// attack tables
pub const PAWN_ATTACKS :[[u64; 64] ; 2] = initialize_pawn_attack_table();
pub const KNIGHT_ATTACKS : [u64; 64] = initialize_knight_attack_table();
pub const KING_ATTACKS : [u64; 64] = initialize_king_attack_table();
#[allow(long_running_const_eval)]
pub static BISHOP_ATTACKS_BLOCKERS_CONSIDERED : [[u64; 4096]; 64] = initialize_bishop_attack_table_blockers_considered();
#[allow(long_running_const_eval)]
pub static ROOK_ATTACKS_BLOCKERS_CONSIDERED : [[u64; 4096]; 64] = initialize_rook_attack_table_blockers_considered();


/**************************************************************/
/*                                                            */
/*  Implementations - helper bitboards                        */
/*                                                            */
/**************************************************************/


const fn initialize_mask_file_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut file = 0;
    while file < 8 {
        table[file] = 0x101010101010101 << file;
        file += 1;
    }

    table
}

const fn initialize_mask_rank_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut rank = 0;
    while rank < 8 {
        table[rank] = 0xFF << (rank * 8);
        rank += 1;
    }

    table
}

const fn initialize_clear_file_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut file = 0;
    while file < 8 {
        table[file] = (0x101010101010101 << file) ^ 0xFFFFFFFFFFFFFFFF;
        file += 1;
    }

    table
}

const fn initialize_clear_rank_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut rank = 0;
    while rank < 8 {
        table[rank] = (0xFF << (rank * 8)) ^ 0xFFFFFFFFFFFFFFFF;
        rank += 1;
    }

    table
}

const fn initialize_mask_anti_diag_table() -> [u64; 15] {
    let mut table = [0; 15];

    let mut diag = 0;
    while diag < 15 {
        let mut sq = 0;
        while sq < 64 {
            if (sq / 8) + (sq % 8) == diag {
                table[diag] |= 1 << sq;
            }
            sq += 1;
        }
        diag += 1;
    }

    table
}

const fn initialize_mask_diag_table() -> [u64; 15] {
    let mut table = [0; 15];

    let mut diag = 0;
    while diag < 15 {
        let mut sq = 0;
        while sq < 64 {
            if (sq / 8) + (7-(sq % 8)) == diag {
                table[diag] |= 1 << sq;
            }
            sq += 1;
        }
        diag += 1;
    }

    table
}

const fn initialize_square_bb_table() -> [u64; 65] {
    let mut table = [0; 65];

    let mut sq = 0;
    while sq < 64 {
        table[sq] = 1 << sq;
        sq += 1;
    }

    table[64] = 0;

    table
}

const fn initialize_squares_between_bb_table() -> [[u64; 64]; 64] {
    let mut table = [[0; 64]; 64];

    let mut sq1 = 0;
    while sq1 < 64 {
        let mut sq2 = 0;
        while sq2 < 64 {
            let sqs = SQUARE_BB[sq1 as usize] | SQUARE_BB[sq2 as usize];
            
            if sq1 != sq2 && (rank_of(sq1) == rank_of(sq2) || file_of(sq1) == file_of(sq2)){
                table[sq1 as usize][sq2 as usize] = (attacks_on_line(sq1, sqs, MASK_RANK[(sq1/8) as usize]) | attacks_on_line(sq1, sqs, MASK_FILE[(sq1%8) as usize])) 
                                                                & (attacks_on_line(sq2, sqs, MASK_RANK[(sq2/8) as usize]) | attacks_on_line(sq2, sqs, MASK_FILE[(sq2%8) as usize]));
            } else if sq1 != sq2 && (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2)) {
                table[sq1 as usize][sq2 as usize] = (attacks_on_line(sq1, sqs, MASK_DIAG[diagonal_of(sq1) as usize]) | attacks_on_line(sq1, sqs, MASK_ANTI_DIAG[anti_diagonal_of(sq1) as usize])) 
                                                                & (attacks_on_line(sq2, sqs, MASK_DIAG[diagonal_of(sq2) as usize]) | attacks_on_line(sq2, sqs, MASK_ANTI_DIAG[anti_diagonal_of(sq2) as usize]));
            }

            sq2 += 1;
        }
        sq1 += 1;
    }

    table
}

const fn initialize_line_spanned_bb_table() -> [[u64; 64]; 64] {
    let mut table = [[0; 64]; 64];

    let mut sq1 = 0;
    while sq1 < 64 {
        let mut sq2 = 0;
        while sq2 < 64 {
            if sq1 != sq2 && (rank_of(sq1) == rank_of(sq2) || file_of(sq1) == file_of(sq2)){
                table[sq1 as usize][sq2 as usize] =  ((attacks_on_line(sq1, 0, MASK_RANK[(sq1/8) as usize]) | attacks_on_line(sq1, 0, MASK_FILE[(sq1%8) as usize])) 
                                                    & (attacks_on_line(sq2, 0, MASK_RANK[(sq2/8) as usize]) | attacks_on_line(sq2, 0, MASK_FILE[(sq2%8) as usize])))
                                                    | (SQUARE_BB[sq1 as usize] | SQUARE_BB[sq2 as usize]);
            } else if sq1 != sq2 && (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2)) {
                table[sq1 as usize][sq2 as usize] =  ((attacks_on_line(sq1, 0, MASK_DIAG[diagonal_of(sq1) as usize]) | attacks_on_line(sq1, 0, MASK_ANTI_DIAG[anti_diagonal_of(sq1) as usize])) 
                                                    & (attacks_on_line(sq2, 0, MASK_DIAG[diagonal_of(sq2) as usize]) | attacks_on_line(sq2, 0, MASK_ANTI_DIAG[anti_diagonal_of(sq2) as usize])))
                                                    | (SQUARE_BB[sq1 as usize] | SQUARE_BB[sq2 as usize]);
            }

            sq2 += 1;
        }
        sq1 += 1;
    }

    table
}

/**************************************************************/
/*                                                            */
/*  Implementations - magic bitboard helper arrays            */
/*                                                            */
/**************************************************************/

const fn initialize_rook_bits_table() -> [u64; 64] {
    

    [12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10,
    10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10,
    10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10,
    11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
    10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12]
}

const fn initialize_bishop_bits_table() -> [u64; 64] {
    

    [6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6]
}

const fn initialize_rook_magic_table() -> [u64; 64] {
    // precaculated magic numbers for rook attacks

    

    [0xa8002c000108020,
    0x4440200140003000,
    0x8080200010011880,
    0x380180080141000,
    0x1a00060008211044,
    0x410001000a0c0008,
    0x9500060004008100,
    0x100024284a20700,
    0x802140008000,
    0x80c01002a00840,
    0x402004282011020,
    0x9862000820420050,
    0x1001448011100,
    0x6432800200800400,
    0x40100010002000c,
    0x2800d0010c080,
    0x90c0008000803042,
    0x4010004000200041,
    0x3010010200040,
    0xa40828028001000,
    0x123010008000430,
    0x24008004020080,
    0x60040001104802,
    0x582200028400d1,
    0x4000802080044000,
    0x408208200420308,
    0x610038080102000,
    0x3601000900100020,
    0x80080040180,
    0xc2020080040080,
    0x80084400100102,
    0x4022408200014401,
    0x40052040800082,
    0xb08200280804000,
    0x8a80a008801000,
    0x4000480080801000,
    0x911808800801401,
    0x822a003002001894,
    0x401068091400108a,
    0x4a10a00004c,
    0x2000800640008024,
    0x1486408102020020,
    0x100a000d50041,
    0x810050020b0020,
    0x204000800808004,
    0x20048100a000c,
    0x112000831020004,
    0x9000040810002,
    0x440490200208200,
    0x8910401000200040,
    0x6404200050008480,
    0x4b824a2010010100,
    0x4080801810c0080,
    0x400802a0080,
    0x8224080110026400,
    0x40002c4104088200,
    0x1002100104a0282,
    0x1208400811048021,
    0x3201014a40d02001,
    0x5100019200501,
    0x101000208001005,
    0x2008450080702,
    0x1002080301d00c,
    0x410201ce5c030092]
}

const fn initialize_bishop_magic_table() -> [u64; 64] {
    // precaculated magic numbers for bishop attacks

    

    [0x40210414004040,
    0x2290100115012200,
    0xa240400a6004201,
    0x80a0420800480,
    0x4022021000000061,
    0x31012010200000,
    0x4404421051080068,
    0x1040882015000,
    0x8048c01206021210,
    0x222091024088820,
    0x4328110102020200,
    0x901cc41052000d0,
    0xa828c20210000200,
    0x308419004a004e0,
    0x4000840404860881,
    0x800008424020680,
    0x28100040100204a1,
    0x82001002080510,
    0x9008103000204010,
    0x141820040c00b000,
    0x81010090402022,
    0x14400480602000,
    0x8a008048443c00,
    0x280202060220,
    0x3520100860841100,
    0x9810083c02080100,
    0x41003000620c0140,
    0x6100400104010a0,
    0x20840000802008,
    0x40050a010900a080,
    0x818404001041602,
    0x8040604006010400,
    0x1028044001041800,
    0x80b00828108200,
    0xc000280c04080220,
    0x3010020080880081,
    0x10004c0400004100,
    0x3010020200002080,
    0x202304019004020a,
    0x4208a0000e110,
    0x108018410006000,
    0x202210120440800,
    0x100850c828001000,
    0x1401024204800800,
    0x41028800402,
    0x20642300480600,
    0x20410200800202,
    0xca02480845000080,
    0x140c404a0080410,
    0x2180a40108884441,
    0x4410420104980302,
    0x1108040046080000,
    0x8141029012020008,
    0x894081818082800,
    0x40020404628000,
    0x804100c010c2122,
    0x8168210510101200,
    0x1088148121080,
    0x204010100c11010,
    0x1814102013841400,
    0xc00010020602,
    0x1045220c040820,
    0x12400808070840,
    0x2004012a040132]
}

const fn initialize_bishop_mask_table() -> [u64; 64] {
    let mut table = [0; 64];

    let mut sq : i32 = 0;
    while sq < 64 {
        let row :i32 = sq / 8;
        let col :i32 = sq % 8;

        // north east
        let mut r = row + 1;
        let mut c = col + 1;
        while r <= 6 && c <= 6 {
            table[sq as usize] |= 1 << (r * 8 + c);
            r += 1;
            c += 1;
        }

        // north west
        r = row + 1;
        c = col - 1;
        while r <= 6 && c >= 1 {
            table[sq as usize] |= 1 << (r * 8 + c);
            r += 1;
            c -= 1;
        }

        // south east
        r = row - 1;
        c = col + 1;
        while r >= 1 && c <= 6 {
            table[sq as usize] |= 1 << (r * 8 + c);
            r -= 1;
            c += 1;
        }

        // south west
        r = row - 1;
        c = col - 1;
        while r >= 1 && c >= 1 {
            table[sq as usize] |= 1 << (r * 8 + c);
            r -= 1;
            c -= 1;
        }

        sq += 1;
    }

    table
}

const fn initialize_rook_mask_table() -> [u64; 64] {
    let mut table = [0; 64];

    let mut sq : i32 = 0;
    while sq < 64 {
        let row :i32 = sq / 8;
        let col :i32 = sq % 8;

        // north
        let mut r = row + 1;
        while r <= 6 {
            table[sq as usize] |= 1 << (r * 8 + col);
            r += 1;
        }

        // south
        r = row - 1;
        while r >= 1 {
            table[sq as usize] |= 1 << (r * 8 + col);
            r -= 1;
        }

        // east
        let mut c = col + 1;
        while c <= 6 {
            table[sq as usize] |= 1 << (row * 8 + c);
            c += 1;
        }

        // west
        c = col - 1;
        while c >= 1 {
            table[sq as usize] |= 1 << (row * 8 + c);
            c -= 1;
        }

        sq += 1;
    }

    table
}


/**************************************************************/
/*                                                            */
/*  Implementations - attack tables                           */
/*                                                            */
/**************************************************************/

const fn initialize_pawn_attack_table() -> [[u64; 64]; 2] {
    let mut table = [[0; 64]; 2];

    let mut sq = 0;
    while sq < 56 {
        let pawn = 1 << sq;
        let attacks = (pawn & CLEAR_FILE[0]) << 7 | (pawn & CLEAR_FILE[7]) << 9;
        table[Player::White as usize][sq] = attacks;
        sq += 1;
    }

    let mut sq = 63;
    while sq >= 8 {
        let pawn = 1 << sq;
        let attacks = (pawn & CLEAR_FILE[0]) >> 9 | (pawn & CLEAR_FILE[7]) >> 7;
        table[Player::Black as usize][sq] = attacks;
        sq -= 1;
    }

    table
}

const fn initialize_knight_attack_table() -> [u64; 64] {
    let mut table = [0; 64];

    let mut sq = 0;
    while sq < 64 {
        let knight = 1 << sq;
        let attacks = ((knight & CLEAR_FILE[0] & CLEAR_FILE[1]) << 6 | (knight & CLEAR_FILE[0] & CLEAR_FILE[1]) >> 10) |
                      ((knight & CLEAR_FILE[6] & CLEAR_FILE[7]) << 10 | (knight & CLEAR_FILE[6] & CLEAR_FILE[7]) >> 6) |
                      ((knight & CLEAR_FILE[0]) << 15 | (knight & CLEAR_FILE[0]) >> 17) |
                      ((knight & CLEAR_FILE[7]) << 17 | (knight & CLEAR_FILE[7]) >> 15);
        table[sq] = attacks;
        sq += 1;
    }

    table
}

const fn initialize_king_attack_table() -> [u64; 64] {
    let mut table = [0; 64];

    let mut sq = 0;
    while sq < 64 {
        let king = 1 << sq;
        
        let attacks = ((king & CLEAR_FILE[0]) >> 1 | (king & CLEAR_FILE[0]) >> 9 | (king & CLEAR_FILE[0]) << 7) |
                      ((king & CLEAR_FILE[7]) << 1 | (king & CLEAR_FILE[7]) << 9 | (king & CLEAR_FILE[7]) >> 7) |
                      ((king >> 8) | (king << 8));

        table[sq] = attacks;
        sq += 1;
    }

    table
}

const fn initialize_bishop_attack_table_blockers_considered() -> [[u64; 4096]; 64] {
    const fn index_to_bitboard(index : i32, n : i32, mask : u64) -> u64 {
        let mut mask = mask;
        let mut j;
        let mut blocking_mask :u64 = 0;
        let mut i = 0;
        while i < n {
            /* pop fist bit */
            if mask == 0 {
                panic!("pop_first_bit called on empty bitboard");
            }
        
            j = first_bit(mask);
            mask &= mask - 1;
    
            /* if i'th bit in number (index) is set */
            if (index & (1 << i)) != 0 {
                /* then carry over i'th 1-bit (namely the j'th bit in mask) from
                 * mask to blocking mask */
                blocking_mask |= 1 << j;
            }
            i += 1;
        }
    
        blocking_mask
    }

    const fn bishop_attacks(sq : u8, block : u64) -> u64 {
        let mut attacks = 0;
        let row :i32 = (sq / 8) as i32;
        let col :i32 = (sq % 8) as i32;
    
        /* north east */
        let mut r : i32= row + 1;
        let mut f: i32 = col + 1;
        while r <= 7 && f <= 7 {
            attacks |= 1 << (f + r * 8);
            if (block & (1 << (f + r * 8))) != 0 {
                break;
            }
            r += 1;
            f += 1;
        }
    
        /* north west */
        r = row + 1;
        f = col - 1;
        while r <= 7 && f >= 0 {
            attacks |= 1 << (f + r * 8);
            if (block & (1 << (f + r * 8))) != 0 {
                break;
            }
            r += 1;
            f -= 1;
        }
    
        /* south east */
        r = row - 1;
        f = col + 1;
        while r >= 0 && f <= 7 {
            attacks |= 1 << (f + r * 8);
            if (block & (1 << (f + r * 8))) != 0 {
                break;
            }
            r -= 1;
            f += 1;
        }
    
        /* south west */
        r = row - 1;
        f = col - 1;
        while r >= 0 && f >= 0 {
            attacks |= 1 << (f + r * 8);
            if (block & (1 << (f + r * 8))) != 0 {
                break;
            }
            r -= 1;
            f -= 1;
        }
    
        attacks
    }

    
    let mut table = [[0; 4096]; 64];

    let mut sq = 0;
    while sq < 64 {
        let mask = BISHOP_MASK[sq];
        let mut i = 0;
        while i < (1 << BISHOP_BITS[sq]) {
            let blockermap = index_to_bitboard(i, BISHOP_BITS[sq] as i32, mask);
            let j = transform(blockermap, BISHOP_MAGIC[sq], BISHOP_BITS[sq] as i32);
            table[sq][j as usize] = bishop_attacks(sq as u8, blockermap);
            i += 1;
        }
        sq += 1;
    }

    table
}

const fn initialize_rook_attack_table_blockers_considered() -> [[u64; 4096]; 64] {
    const fn index_to_bitboard(index : i32, n : i32, mask : u64) -> u64 {
        let mut mask = mask;
        let mut j;
        let mut blocking_mask :u64 = 0;
        let mut i = 0;
        while i < n {
            /* pop fist bit */
            if mask == 0 {
                panic!("pop_first_bit called on empty bitboard");
            }
        
            j = first_bit(mask);
            mask &= mask - 1;
    
            /* if i'th bit in number (index) is set */
            if (index & (1 << i)) != 0 {
                /* then carry over i'th 1-bit (namely the j'th bit in mask) from
                 * mask to blocking mask */
                blocking_mask |= 1 << j;
            }
            i += 1;
        }
    
        blocking_mask
    }

    const fn rook_attacks(sq : u8, block : u64) -> u64 {
        let mut attacks = 0;
        let row :i32 = (sq / 8) as i32;
        let col :i32 = (sq % 8) as i32;
    
        /* north */
        let mut r : i32= row + 1;
        while r <= 7 {
            attacks |= 1 << (col + r * 8);
            if (block & (1 << (col + r * 8))) != 0 {
                break;
            }
            r += 1;
        }
    
        /* south */
        r = row - 1;
        while r >= 0 {
            attacks |= 1 << (col + r * 8);
            if (block & (1 << (col + r * 8))) != 0 {
                break;
            }
            r -= 1;
        }
    
        /* east */
        let mut f: i32 = col + 1;
        while f <= 7 {
            attacks |= 1 << (f + row * 8);
            if (block & (1 << (f + row * 8))) != 0 {
                break;
            }
            f += 1;
        }
    
        /* west */
        f = col - 1;
        while f >= 0 {
            attacks |= 1 << (f + row * 8);
            if (block & (1 << (f + row * 8))) != 0 {
                break;
            }
            f -= 1;
        }
    
        attacks
    }

    let mut table = [[0; 4096]; 64];

    let mut sq = 0;
    while sq < 64 {
        let mask = ROOK_MASK[sq];
        let mut i = 0;
        while i < (1 << ROOK_BITS[sq]) {
            let blockermap = index_to_bitboard(i, ROOK_BITS[sq] as i32, mask);
            let j = transform(blockermap, ROOK_MAGIC[sq], ROOK_BITS[sq] as i32);
            table[sq][j as usize] = rook_attacks(sq as u8, blockermap);
            i += 1;
        }
        sq += 1;
    }

    table
}

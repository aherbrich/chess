use crate::tables::{SQUARE_BB, BISHOP_MASK, BISHOP_MAGIC, BISHOP_BITS, BISHOP_ATTACKS_BLOCKERS_CONSIDERED, ROOK_MASK, ROOK_MAGIC, ROOK_BITS, ROOK_ATTACKS_BLOCKERS_CONSIDERED};

pub const fn rank_of(sq : u8) -> u8 {
    sq >> 3
}

pub const fn file_of(sq : u8) -> u8 {
    sq & 7
}

pub const fn diagonal_of(sq : u8) -> u8 {
    rank_of(sq) + 7 - file_of(sq)
}

pub const fn anti_diagonal_of(sq : u8) -> u8 {
    rank_of(sq) + file_of(sq)
}

pub const fn attacks_on_line(sq : u8, occ : u64, mask : u64) -> u64 {
    // calculates sliding attacks from a given square, on a given line (vertical, horizontal ....)
    // taking into account the blocking pieces. 
    // calculation is based on the 'hyperbola quintessence algorithm'
    //
    //     square       ,        occupied       ,       mask            =       attacks
    // - - - - - - - -      - - - - - o - -         - - - - - x - -         - - - - - - - -
    // - - - - - - - -      - - - - - o - -         - - - - - x - -         - - - - - a - -
    // - - - - - - - -      - - - - - - - -         - - - - - x - -         - - - - - a - -
    // - - - - - - - -      - - - - - - - -         - - - - - x - -         - - - - - a - -
    // - - - - s - - -  ,   - - - - - - - -     ,   - - - - - x - -     =   - - - - - - - -
    // - - - - - - - -      - - - - - - - -         - - - - - x - -         - - - - - a - -
    // - - - - - - - -      - - - - - o - -         - - - - - x - -         - - - - - a - -
    // - - - - - - - -      - - - - - - - -         - - - - - x - -         - - - - - - - -

    const fn reverse(bb : u64) -> u64 {
        let mut bb = bb;
        bb = ((bb & 0x5555555555555555) << 1) | ((bb >> 1) & 0x5555555555555555);
        bb = ((bb & 0x3333333333333333) << 2) | ((bb >> 2) & 0x3333333333333333);
        bb = ((bb & 0x0f0f0f0f0f0f0f0f) << 4) | ((bb >> 4) & 0x0f0f0f0f0f0f0f0f);
        bb = ((bb & 0x00ff00ff00ff00ff) << 8) | ((bb >> 8) & 0x00ff00ff00ff00ff);

        (bb << 48) | ((bb & 0xffff0000) << 16) | ((bb >> 16) & 0xffff0000) | (bb >> 48)
    }

    (((mask & occ).wrapping_sub(SQUARE_BB[sq as usize].wrapping_mul(2))) ^ reverse(reverse(mask & occ).wrapping_sub(reverse(SQUARE_BB[sq as usize]).wrapping_mul(2)))) & mask
}

pub const fn transform(mask : u64, magic : u64, bits : i32) -> i32 {
    /* depending on the blockers mask, we need a different key to access the
     * correct attack map */
    (mask.wrapping_mul(magic)  >> (64 - bits)) as i32
}

pub fn attacks_bishop(sq : u8, occ : u64) -> u64 {
    let j = transform(occ & BISHOP_MASK[sq as usize], BISHOP_MAGIC[sq as usize], BISHOP_BITS[sq as usize] as i32);
    BISHOP_ATTACKS_BLOCKERS_CONSIDERED[sq as usize][j as usize]
}

pub fn attacks_rook(sq : u8, occ : u64) -> u64 {
    let j = transform(occ & ROOK_MASK[sq as usize], ROOK_MAGIC[sq as usize], ROOK_BITS[sq as usize] as i32);
    ROOK_ATTACKS_BLOCKERS_CONSIDERED[sq as usize][j as usize]
}

pub const fn first_bit(bb : u64) -> u8 {
    const DEBRUIJN_TABLE : [u8; 64] = 
    [0, 47, 1, 56, 48, 27, 2, 60,
    57, 49, 41, 37, 28, 16, 3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11, 4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30, 9, 24,
    13, 18, 8, 12, 7, 6, 5, 63];
    const DEBRUIJN_MAGIC : u64 = 0x03f79d71b4cb0a89;
    let num = bb ^ (bb - 1);
    DEBRUIJN_TABLE[(num.wrapping_mul(DEBRUIJN_MAGIC) >> 58) as usize]
}

pub fn pop_first_bit(bb : &mut u64) -> u8 {
    if *bb == 0 {
        panic!("pop_first_bit called on empty bitboard");
    }

    let bit = first_bit(*bb);
    *bb &= *bb - 1;
    bit
}

pub fn sparse_pop_count(mut bb : u64) -> u8 {
    let mut count = 0;
    while bb != 0 {
        count += 1;
        bb &= bb.wrapping_sub(1);
    }
    count
}



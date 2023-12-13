use core::{panic, fmt};

const DEBRUIJN_TABLE : [u8; 64] = 
[0, 47, 1, 56, 48, 27, 2, 60,
57, 49, 41, 37, 28, 16, 3, 61,
54, 58, 35, 52, 50, 42, 21, 44,
38, 32, 29, 23, 17, 11, 4, 62,
46, 55, 26, 59, 40, 36, 15, 53,
34, 51, 20, 43, 31, 22, 10, 45,
25, 39, 14, 33, 19, 30, 9, 24,
13, 18, 8, 12, 7, 6, 5, 63];

const FIELD : [&str; 64] = ["a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2",
"c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3",
"e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4",
"g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7",
"c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8",
"e8", "f8", "g8", "h8"];

const CASTLE_WHITE_KING : u8 = 1;
const CASTLE_WHITE_QUEEN : u8 = 2;
const CASTLE_BLACK_KING : u8 = 4;
const CASTLE_BLACK_QUEEN : u8 = 8;

const NO_SQUARE : u8 = 64;
const NO_PIECE : u8 = 255;

// helper bitboards 
const MASK_FILE : [u64; 8] = initialize_mask_file_table();
const MASK_RANK : [u64; 8] = initialize_mask_rank_table();
const CLEAR_FILE : [u64; 8] = initialize_clear_file_table();
const CLEAR_RANK : [u64; 8] = initialize_clear_rank_table();
const MASK_ANTI_DIAG : [u64; 15] = initialize_mask_anti_diag_table();
const MASK_DIAG : [u64; 15] = initialize_mask_diag_table();
const SQUARE_BB : [u64; 65] = initialize_square_bb_table();
const UNIBOARD : u64 = 0xFFFFFFFFFFFFFFFF;
const SQUARES_BETWEEN_BB : [[u64; 64]; 64] = initialize_squares_between_bb_table();
const LINE_SPANNED_BB : [[u64; 64]; 64] = initialize_line_spanned_bb_table();

// magic bitboard helper arrays for sliding pieces
const ROOK_BITS : [u64; 64] = initialize_rook_bits_table();
const BISHOP_BITS : [u64; 64] = initialize_bishop_bits_table();
const ROOK_MAGIC : [u64; 64] = initialize_rook_magic_table();
const BISHOP_MAGIC : [u64; 64] = initialize_bishop_magic_table();
const ROOK_MASK : [u64; 64] = initialize_rook_mask_table();
const BISHOP_MASK : [u64; 64] = initialize_bishop_mask_table();

// attack tables
const PAWN_ATTACKS :[[u64; 64] ; 2] = initialize_pawn_attack_table();
const KNIGHT_ATTACKS : [u64; 64] = initialize_knight_attack_table();
const KING_ATTACKS : [u64; 64] = initialize_king_attack_table();
#[allow(long_running_const_eval)]
static BISHOP_ATTACKS_BLOCKERS_CONSIDERED : [[u64; 4096]; 64] = initialize_bishop_attack_table_blockers_considered();
#[allow(long_running_const_eval)]
static ROOK_ATTACKS_BLOCKERS_CONSIDERED : [[u64; 4096]; 64] = initialize_rook_attack_table_blockers_considered();

enum Direction {
    North = 8,
    NorthNorth = 16,
    South = -8,
    SouthSouth = -16,
    East = 1,
    West = -1,
    NorthEast = 9,
    NorthWest = 7,
    SouthEast = -7,
    SouthWest = -9
}

enum File {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H
}

enum Rank {
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight
}

enum MoveFlags {
    Quiet = 0,
    DoublePawnPush = 1,
    KingCastle = 2,
    QueenCastle = 3,
    Capture = 4,
    EnPassant = 5,
    KnightPromo = 8,
    BishopPromo = 9,
    RookPromo = 10,
    QueenPromo = 11,
    KnightPromoCapture = 12,
    BishopPromoCapture = 13,
    RookPromoCapture = 14,
    QueenPromoCapture = 15
}

impl MoveFlags {
    fn from_u8(val : u8) -> MoveFlags {
        match val {
            0 => MoveFlags::Quiet,
            1 => MoveFlags::DoublePawnPush,
            2 => MoveFlags::KingCastle,
            3 => MoveFlags::QueenCastle,
            4 => MoveFlags::Capture,
            5 => MoveFlags::EnPassant,
            8 => MoveFlags::KnightPromo,
            9 => MoveFlags::BishopPromo,
            10 => MoveFlags::RookPromo,
            11 => MoveFlags::QueenPromo,
            12 => MoveFlags::KnightPromoCapture,
            13 => MoveFlags::BishopPromoCapture,
            14 => MoveFlags::RookPromoCapture,
            15 => MoveFlags::QueenPromoCapture,
            _ => panic!("Invalid MoveFlags value")
        }
    }
}

#[derive(Copy, Clone, Eq, PartialEq, Debug)]
enum Player {
    White,
    Black
}

#[derive(Copy, Clone)]
enum Piece {
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing
}

impl Piece {
    fn is_pawn(&self) -> bool {
        match self {
            Piece::WhitePawn | Piece::BlackPawn => true,
            _ => false
        }
    }
}

#[derive(Copy, Clone)]
enum Square {
    Empty,
    Occupied(Piece)
}

#[derive(Copy, Clone)]
struct Move {
    from : u8,
    to : u8,
    flags : u8
}

impl fmt::Debug for Move {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut result = String::new();
        let from = FIELD[self.from as usize];
        let to = FIELD[self.to as usize];

        if self.flags >= 8 {
            result.push_str(from);
            result.push_str(to);
            match self.flags {
                8 | 12 => result.push('n'),
                9 | 13 => result.push('b'),
                10 | 14 => result.push('r'),
                11 | 15 => result.push('q'),
                _ => ()
            }
        } else {
            result.push_str(from);
            result.push_str(to);
        }

        result.push_str(&format!(" ({})", self.flags));
        write!(f, "{}", result)

    }
}

impl Move {
    fn new(from : u8, to : u8, flags : MoveFlags) -> Move {
        Move {
            from,
            to,
            flags : flags as u8
        }
    }

    fn print(&self) {
        println!("from: {}, to: {}, flags: {}", self.from, self.to, self.flags);
    }
}
#[derive(Copy, Clone)]
struct UndoInfo {
    castle : u8,
    capture : Square,
    
    epsq : u8,

    fullmove : u16,
    fiftymove : u8,

    hash : u64
}

struct Board {
    playingfield : [Square; 64],
    piece_bitboards : [u64; 12],
    checkers : u64,
    pinned : u64,
    attackmap : u64,
    capture_mask : u64,
    quiet_mask : u64,

    side_to_move : Player,
    ply : u16,
    hash : u64,

    history : [UndoInfo; 1024],

    moves : Vec<Move>,
}

impl Board {
    fn new() -> Board {
        Board {
            playingfield : [Square::Empty; 64],
            piece_bitboards : [0; 12],
            checkers : 0,
            pinned : 0,
            attackmap : 0,
            capture_mask : 0,
            quiet_mask : 0,

            side_to_move : Player::White,
            ply : 0,
            hash : 0,

            history : [UndoInfo {
                castle : 0,
                capture : Square::Empty,
                epsq : NO_SQUARE,
                fullmove : 0,
                fiftymove : 0,
                hash : 0
            }; 1024],
            moves : Vec::new()
            
        }
    }

    fn reset_board(&mut self) {
        self.playingfield = [Square::Empty; 64];
        self.piece_bitboards = [0; 12];
        self.checkers = 0;
        self.pinned = 0;
        self.attackmap = 0;
        self.capture_mask = 0;
        self.quiet_mask = 0;

        self.side_to_move = Player::White;
        self.ply = 0;
        self.hash = 0;

        self.history = [UndoInfo {
            castle : 0,
            capture : Square::Empty,
            epsq : NO_SQUARE,
            fullmove : 0,
            fiftymove : 0,
            hash : 0
        }; 1024];
        self.moves.clear();
    }

    fn set_by_fen(&mut self, fen : &str) {
        // reset board
        self.reset_board();

        let mut rank = 7;
        let mut file = 0;

        // split fen into parts
        let mut fen_parts = fen.split_whitespace();

        // set playingfield by first part
        for c in fen_parts.next().unwrap().chars() {
            match c {
                'P' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhitePawn);
                    self.piece_bitboards[Piece::WhitePawn as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'N' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhiteKnight);
                    self.piece_bitboards[Piece::WhiteKnight as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'B' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhiteBishop);
                    self.piece_bitboards[Piece::WhiteBishop as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'R' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhiteRook);
                    self.piece_bitboards[Piece::WhiteRook as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'Q' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhiteQueen);
                    self.piece_bitboards[Piece::WhiteQueen as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'K' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::WhiteKing);
                    self.piece_bitboards[Piece::WhiteKing as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'p' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackPawn);
                    self.piece_bitboards[Piece::BlackPawn as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'n' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackKnight);
                    self.piece_bitboards[Piece::BlackKnight as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'b' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackBishop);
                    self.piece_bitboards[Piece::BlackBishop as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'r' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackRook);
                    self.piece_bitboards[Piece::BlackRook as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'q' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackQueen);
                    self.piece_bitboards[Piece::BlackQueen as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                'k' => {
                    self.playingfield[rank * 8 + file] = Square::Occupied(Piece::BlackKing);
                    self.piece_bitboards[Piece::BlackKing as usize] |= 1 << (rank * 8 + file);
                    file += 1;
                },
                '/' => {
                    rank -= 1;
                    file = 0;
                },
                _ => {
                    file += c.to_digit(10).unwrap() as usize;
                }

            }
        }

        // set side to move by second part
        match fen_parts.next().unwrap() {
            "w" => self.side_to_move = Player::White,
            "b" => self.side_to_move = Player::Black,
            _ => ()
        }

        // set castling rights by third part
        for c in fen_parts.next().unwrap().chars() {
            match c {
                'K' => self.history[0].castle |= CASTLE_WHITE_KING,
                'Q' => self.history[0].castle |= CASTLE_WHITE_QUEEN,
                'k' => self.history[0].castle |= CASTLE_BLACK_KING,
                'q' => self.history[0].castle |= CASTLE_BLACK_QUEEN,
                _ => ()
            }
        }

        // set en passant square by fourth part
        match fen_parts.next().unwrap() {
            "-" => self.history[0].epsq = NO_SQUARE,
            field => {
                let file = field.chars().nth(0).unwrap() as i32 - 'a' as i32;
                let rank = field.chars().nth(1).unwrap() as i32 - '1' as i32;
                self.history[0].epsq = (rank * 8 + file) as u8;
            }
        }

        // set fiftymove counter by fifth part
        self.history[0].fiftymove = fen_parts.next().unwrap().parse::<u8>().unwrap();

        // set fullmove number by sixth part
        self.history[0].fullmove = fen_parts.next().unwrap().parse::<u16>().unwrap();

        // TODO hashes
    }
    
    fn print(&self) {
        for rank in (0..8).rev() {
            for file in 0..8 {
                match self.playingfield[rank * 8 + file] {
                    Square::Occupied(Piece::WhitePawn) => print!("P"),
                    Square::Occupied(Piece::WhiteKnight) => print!("N"),
                    Square::Occupied(Piece::WhiteBishop) => print!("B"),
                    Square::Occupied(Piece::WhiteRook) => print!("R"),
                    Square::Occupied(Piece::WhiteQueen) => print!("Q"),
                    Square::Occupied(Piece::WhiteKing) => print!("K"),
                    Square::Occupied(Piece::BlackPawn) => print!("p"),
                    Square::Occupied(Piece::BlackKnight) => print!("n"),
                    Square::Occupied(Piece::BlackBishop) => print!("b"),
                    Square::Occupied(Piece::BlackRook) => print!("r"),
                    Square::Occupied(Piece::BlackQueen) => print!("q"),
                    Square::Occupied(Piece::BlackKing) => print!("k"),
                    Square::Empty => print!("-")
                }
                print!(" ")
            }
            println!();
        }
        println!("Side to move: {:?}", self.side_to_move);

        print_bitboard(&self.piece_bitboards[Piece::WhitePawn as usize]);
        print_bitboard(&self.piece_bitboards[Piece::WhiteKnight as usize]);
        print_bitboard(&self.piece_bitboards[Piece::WhiteBishop as usize]);
        print_bitboard(&self.piece_bitboards[Piece::WhiteRook as usize]);
        print_bitboard(&self.piece_bitboards[Piece::WhiteQueen as usize]);
        print_bitboard(&self.piece_bitboards[Piece::WhiteKing as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackPawn as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackKnight as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackBishop as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackRook as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackQueen as usize]);
        print_bitboard(&self.piece_bitboards[Piece::BlackKing as usize]);
    }

    fn check(&self) {
        for i in 0..64 {
            match self.playingfield[i] {
                Square::Occupied(Piece::WhitePawn) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white pawn");
                        }
                },
                Square::Occupied(Piece::WhiteKnight) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white knight");
                        }
                },
                Square::Occupied(Piece::WhiteBishop) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white bishop");
                        }
                },
                Square::Occupied(Piece::WhiteRook) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white rook");
                        }
                },
                Square::Occupied(Piece::WhiteQueen) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white queen");
                        }
                },
                Square::Occupied(Piece::WhiteKing) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - white king");
                        }
                },
                Square::Occupied(Piece::BlackPawn) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) == 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - black pawn");
                        }
                },
                Square::Occupied(Piece::BlackKnight) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - black knight");
                        }
                },
                Square::Occupied(Piece::BlackBishop) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - black bishop");
                        }
                },
                Square::Occupied(Piece::BlackRook) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - black rook");
                        }
                },
                Square::Occupied(Piece::BlackQueen) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) == 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - black queen");
                        }
                },
                Square::Occupied(Piece::BlackKing) => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) == 0 {
                            self.print();
                            panic!("Invalid board - black king");
                        }
                },
                Square::Empty => {
                    if  (self.piece_bitboards[Piece::WhitePawn as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackPawn as usize] & SQUARE_BB[i]) != 0  ||
                        (self.piece_bitboards[Piece::WhiteKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKnight as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackBishop as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackRook as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackQueen as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::WhiteKing as usize] & SQUARE_BB[i]) != 0 ||
                        (self.piece_bitboards[Piece::BlackKing as usize] & SQUARE_BB[i]) != 0 {
                            self.print();
                            panic!("Invalid board - empty square");
                        }
                }
            }
        }
    }

    fn print_moves(&self) {
        println!("{} moves", self.moves.len());
        for mov in &self.moves {
            mov.print();
        }
    }

    fn remove_piece(&mut self, sq : u8) {
        // TODO hashes
        match self.playingfield[sq as usize] {
            Square::Occupied(piece) => {
                self.piece_bitboards[piece as usize] &= !SQUARE_BB[sq as usize];
            },
            _ => ()
        };
        self.playingfield[sq as usize] = Square::Empty;
    }

    fn put_piece(&mut self, sq : u8, pc : Square) {
        match pc {
            Square::Occupied(piece) => {
                self.piece_bitboards[piece as usize] |= SQUARE_BB[sq as usize];
            },
            _ => ()
        }
        self.playingfield[sq as usize] = pc;
        // TODO hashes
    }

    fn move_piece (&mut self, from : u8, to : u8) {
        // TODO hashes
        let mask = SQUARE_BB[from as usize] | SQUARE_BB[to as usize];
        
        match self.playingfield[from as usize] {
            Square::Occupied(piece) => {
                self.piece_bitboards[piece as usize] ^= mask;
            },
            _ => { 
                self.print();
                panic!("No piece on square {} {}", from, to);
            }
        }

        match self.playingfield[to as usize] {
            Square::Occupied(piece) => {
                self.piece_bitboards[piece as usize] &= !mask;
            },
            _ => {
                self.print();
                print_bitboard(&self.capture_mask);
                print_bitboard(&self.piece_bitboards[Piece::WhitePawn as usize]);
                print_bitboard(&self.piece_bitboards[Piece::BlackPawn as usize]);
                print_bitboard(&self.piece_bitboards[Piece::WhiteKnight as usize]);
                print_bitboard(&self.piece_bitboards[Piece::BlackKnight as usize]);

                panic!("No piece on square {} {}", from, to);
            }
        }

        self.playingfield[to as usize] = self.playingfield[from as usize];
        self.playingfield[from as usize] = Square::Empty;
    }

    fn move_piece_quiet(&mut self, from : u8, to : u8) {
        // TODO hashes
        match self.playingfield[from as usize] {
            Square::Occupied(piece) => {
                self.piece_bitboards[piece as usize] ^= SQUARE_BB[from as usize] | SQUARE_BB[to as usize];
            },
            _ => {
                self.print();
                panic!("No piece on square {} {}", from, to);
            }
        }

        self.playingfield[to as usize] = self.playingfield[from as usize];
        self.playingfield[from as usize] = Square::Empty;
    }

    fn do_move(&mut self, mov : Move) {
        // save current board hash in array
        self.history[self.ply as usize].hash = self.hash;

        // increase ply
        self.ply += 1;
        let ply = self.ply;

        // reset history entry at new ply
        self.history[ply as usize].capture = Square::Empty;
        self.history[ply as usize].epsq = NO_SQUARE;
        self.history[ply as usize].castle = self.history[(ply - 1) as usize].castle;
        self.history[ply as usize].fiftymove = self.history[(ply - 1) as usize].fiftymove;
        self.history[ply as usize].fullmove = self.history[(ply - 1) as usize].fullmove;

        // adjustment of hash
        // XOR out (old) ep square if an ep sqaure was set i.e. ep capture was possible at ply - 1
        if self.history[(ply - 1) as usize].epsq != NO_SQUARE {
            // TODO hashes
        }
        // XOR out (old) castling rights
        // TODO hashes

        // reset fifty move counter if move is a pawn move or a capture
        if mov.flags & 0b100 != 0 || (SQUARE_BB[mov.from as usize] & self.piece_bitboards[Piece::WhitePawn as usize] != 0) ||
           (SQUARE_BB[mov.from as usize] & self.piece_bitboards[Piece::BlackPawn as usize] != 0) {
            self.history[ply as usize].fiftymove = 0;
        } else {
            self.history[ply as usize].fiftymove = self.history[(ply - 1) as usize].fiftymove + 1;
        }

        // if black is moving, increase fullmove counter
        if self.side_to_move == Player::Black {
            self.history[ply as usize].fullmove += 1;
        }

        // adjust casteling rights if (potentially) king or rook moved from their initial position
        match mov.from {
            0 => self.history[ply as usize].castle &= !(CASTLE_WHITE_QUEEN),
            7 => self.history[ply as usize].castle &= !(CASTLE_WHITE_KING),
            56 => self.history[ply as usize].castle &= !(CASTLE_BLACK_QUEEN),
            63 => self.history[ply as usize].castle &= !(CASTLE_BLACK_KING),
            4 => self.history[ply as usize].castle &= !(CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN),
            60 => self.history[ply as usize].castle &= !(CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN),
            _ => ()
        }

        // adjust casterling rights if rook were (potentially) captured
        match mov.to {
            0 => self.history[ply as usize].castle &= !(CASTLE_WHITE_QUEEN),
            7 => self.history[ply as usize].castle &= !(CASTLE_WHITE_KING),
            56 => self.history[ply as usize].castle &= !(CASTLE_BLACK_QUEEN),
            63 => self.history[ply as usize].castle &= !(CASTLE_BLACK_KING),
            _ => ()
        }

        match MoveFlags::from_u8(mov.flags){
            MoveFlags::Quiet => {
                self.move_piece_quiet(mov.from, mov.to);
            },
            MoveFlags::DoublePawnPush => {
                self.move_piece_quiet(mov.from, mov.to);
                match self.side_to_move {
                    Player::White => self.history[ply as usize].epsq = mov.from + 8,
                    Player::Black => self.history[ply as usize].epsq = mov.from - 8
                }
                // TODO hashes
            },
            MoveFlags::KingCastle => {
                match self.side_to_move {
                    Player::White => {
                        self.move_piece_quiet(4, 6);
                        self.move_piece_quiet(7, 5);
                        self.history[ply as usize].castle &= !(CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN);                        
                    },
                    Player::Black => {
                        self.move_piece_quiet(60, 62);
                        self.move_piece_quiet(63, 61);
                        self.history[ply as usize].castle &= !(CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN);
                    }
                }
            },
            MoveFlags::QueenCastle => {
                match self.side_to_move {
                    Player::White => {
                        self.move_piece_quiet(4, 2);
                        self.move_piece_quiet(0, 3);
                        self.history[ply as usize].castle &= !(CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN);                        
                    },
                    Player::Black => {
                        self.move_piece_quiet(60, 58);
                        self.move_piece_quiet(56, 59);
                        self.history[ply as usize].castle &= !(CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN);
                    }
                }
            },
            MoveFlags::EnPassant => {
                self.move_piece_quiet(mov.from, mov.to);

                match self.side_to_move {
                    Player::White => {
                        self.history[(ply-1) as usize].capture = Square::Occupied(Piece::BlackPawn);
                        self.remove_piece(mov.to - 8);
                    },
                    Player::Black => {
                        self.history[(ply-1) as usize].capture = Square::Occupied(Piece::WhitePawn);
                        self.remove_piece(mov.to + 8);
                    }
                }
            },
            MoveFlags::KnightPromo => {
                self.remove_piece(mov.from);    
                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteKnight,
                    Player::Black => Piece::BlackKnight
                }));
            },
            MoveFlags::BishopPromo => {
                self.remove_piece(mov.from);    
                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteBishop,
                    Player::Black => Piece::BlackBishop
                }));
            },
            MoveFlags::RookPromo => {
                self.remove_piece(mov.from);    
                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteRook,
                    Player::Black => Piece::BlackRook
                }));
            },
            MoveFlags::QueenPromo => {
                self.remove_piece(mov.from);    
                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteQueen,
                    Player::Black => Piece::BlackQueen
                }));
            },
            MoveFlags::KnightPromoCapture => {
                self.history[(ply-1) as usize].capture = self.playingfield[mov.to as usize];

                self.remove_piece(mov.from);    
                self.remove_piece(mov.to);

                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteKnight,
                    Player::Black => Piece::BlackKnight
                }));
            },
            MoveFlags::BishopPromoCapture => {
                self.history[(ply-1) as usize].capture = self.playingfield[mov.to as usize];

                self.remove_piece(mov.from);    
                self.remove_piece(mov.to);

                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteBishop,
                    Player::Black => Piece::BlackBishop
                }));
            },
            MoveFlags::RookPromoCapture => {
                self.history[(ply-1) as usize].capture = self.playingfield[mov.to as usize];

                self.remove_piece(mov.from);    
                self.remove_piece(mov.to);

                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteRook,
                    Player::Black => Piece::BlackRook
                }));
            },
            MoveFlags::QueenPromoCapture => {
                self.history[(ply-1) as usize].capture = self.playingfield[mov.to as usize];

                self.remove_piece(mov.from);    
                self.remove_piece(mov.to);

                self.put_piece(mov.to, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::WhiteQueen,
                    Player::Black => Piece::BlackQueen
                }));
            },
            MoveFlags::Capture => {
                self.history[(ply-1) as usize].capture = self.playingfield[mov.to as usize];
                self.move_piece(mov.from, mov.to);
            }
        }

        self.side_to_move = match self.side_to_move {
            Player::White => Player::Black,
            Player::Black => Player::White
        };

        // TODO hashes
    }

    fn undo_move(&mut self, mov : Move) {
        // XOR out old castle rights 
        // TODO hashes

        // reduce ply
        self.ply -= 1;

        // XOR in new castle rights
        // TODO hashes

        // XOR in the (old) ep square if an ep square was set i.e. ep capture was possible at ply - 1
        if self.history[self.ply as usize].epsq != NO_SQUARE {
            // TODO hashes
        }

        match MoveFlags::from_u8(mov.flags) {
            MoveFlags::Quiet => {
                self.move_piece_quiet(mov.to, mov.from);
            },
            MoveFlags::DoublePawnPush => {
                /* XOR out the new ep square */
                // TODO hashes
                self.move_piece_quiet(mov.to, mov.from);
            },
            MoveFlags::KingCastle => {
                match self.side_to_move {
                    Player::Black => {
                        self.move_piece_quiet(6, 4);
                        self.move_piece_quiet(5, 7);
                    },
                    Player::White => {
                        self.move_piece_quiet(62, 60);
                        self.move_piece_quiet(61, 63);
                    }
                }
            },
            MoveFlags::QueenCastle => {
                match self.side_to_move {
                    Player::Black => {
                        self.move_piece_quiet(2, 4);
                        self.move_piece_quiet(3, 0);
                    },
                    Player::White => {
                        self.move_piece_quiet(58, 60);
                        self.move_piece_quiet(59, 56);
                    }
                }
            },
            MoveFlags::EnPassant => {
                self.move_piece_quiet(mov.to, mov.from);
                match self.side_to_move {
                    Player::Black => {
                        self.put_piece(mov.to - 8, Square::Occupied(Piece::BlackPawn));
                    },
                    Player::White => {
                        self.put_piece(mov.to + 8, Square::Occupied(Piece::WhitePawn));
                    }
                }
            },
            MoveFlags::KnightPromo | MoveFlags::BishopPromo | MoveFlags::RookPromo | MoveFlags::QueenPromo => {
                self.remove_piece(mov.to);
                self.put_piece(mov.from, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::BlackPawn,
                    Player::Black => Piece::WhitePawn
                }));
            },
            MoveFlags::KnightPromoCapture | MoveFlags::BishopPromoCapture | MoveFlags::RookPromoCapture | MoveFlags::QueenPromoCapture => {
                self.remove_piece(mov.to);
                self.put_piece(mov.from, Square::Occupied(match self.side_to_move {
                    Player::White => Piece::BlackPawn,
                    Player::Black => Piece::WhitePawn
                }));
                self.put_piece(mov.to, self.history[self.ply as usize].capture);
            },
            MoveFlags::Capture => {
                self.move_piece_quiet(mov.to, mov.from);
                self.put_piece(mov.to, self.history[self.ply as usize].capture);
            }
        }

        self.side_to_move = match self.side_to_move {
            Player::White => Player::Black,
            Player::Black => Player::White
        };

        // XOR out the (old) player and XOR in the (new) player
        // TODO hashes
        
    }

    fn diagonal_sliders(&self, player : Player) -> u64 {
        let mut bb = 0;

        if player == Player::White {
            bb |= self.piece_bitboards[Piece::WhiteBishop as usize] | self.piece_bitboards[Piece::WhiteQueen as usize];
        } else {
            bb |= self.piece_bitboards[Piece::BlackBishop as usize] | self.piece_bitboards[Piece::BlackQueen as usize];
        }

        bb
    }

    fn orthogonal_sliders(&self, player : Player) -> u64 {
        let mut bb = 0;

        if player == Player::White {
            bb |= self.piece_bitboards[Piece::WhiteRook as usize] | self.piece_bitboards[Piece::WhiteQueen as usize];
        } else {
            bb |= self.piece_bitboards[Piece::BlackRook as usize] | self.piece_bitboards[Piece::BlackQueen as usize];
        }

        bb
    }

    fn attackers_from(&self, sq : u8, occ : u64, player : Player) -> u64 {
        if player == Player::White {
            (attacks_pawn_single(sq, Player::Black) & self.piece_bitboards[Piece::WhitePawn as usize]) |
            (KNIGHT_ATTACKS[sq as usize] & self.piece_bitboards[Piece::WhiteKnight as usize]) |
            (attacks_bishop(sq, occ) & (self.piece_bitboards[Piece::WhiteBishop as usize] | self.piece_bitboards[Piece::WhiteQueen as usize])) |
            (attacks_rook(sq, occ) & (self.piece_bitboards[Piece::WhiteRook as usize] | self.piece_bitboards[Piece::WhiteQueen as usize]))
        } else {
            (attacks_pawn_single(sq, Player::White) & self.piece_bitboards[Piece::BlackPawn as usize]) |
            (KNIGHT_ATTACKS[sq as usize] & self.piece_bitboards[Piece::BlackKnight as usize]) |
            (attacks_bishop(sq, occ) & (self.piece_bitboards[Piece::BlackBishop as usize] | self.piece_bitboards[Piece::BlackQueen as usize])) |
            (attacks_rook(sq, occ) & (self.piece_bitboards[Piece::BlackRook as usize] | self.piece_bitboards[Piece::BlackQueen as usize]))
        }
    }

    fn generate_moves(&mut self) {
        self.moves.clear();
        
        /***************************************************************************/
        /*                                                                         */
        /* USEFUL BITBOARDS                                                        */
        /*                                                                         */
        /***************************************************************************/
        
        let us = self.side_to_move;
        let them = match us {
            Player::White => Player::Black,
            Player::Black => Player::White
        };

        let us_bb = if us == Player::White { self.piece_bitboards[Piece::WhitePawn as usize] | self.piece_bitboards[Piece::WhiteKnight as usize] | self.piece_bitboards[Piece::WhiteBishop as usize] | self.piece_bitboards[Piece::WhiteRook as usize] | self.piece_bitboards[Piece::WhiteQueen as usize] | self.piece_bitboards[Piece::WhiteKing as usize] } 
                         else { self.piece_bitboards[Piece::BlackPawn as usize] | self.piece_bitboards[Piece::BlackKnight as usize] | self.piece_bitboards[Piece::BlackBishop as usize] | self.piece_bitboards[Piece::BlackRook as usize] | self.piece_bitboards[Piece::BlackQueen as usize] | self.piece_bitboards[Piece::BlackKing as usize] };

        let them_bb = if them == Player::White { self.piece_bitboards[Piece::WhitePawn as usize] | self.piece_bitboards[Piece::WhiteKnight as usize] | self.piece_bitboards[Piece::WhiteBishop as usize] | self.piece_bitboards[Piece::WhiteRook as usize] | self.piece_bitboards[Piece::WhiteQueen as usize] | self.piece_bitboards[Piece::WhiteKing as usize] } 
                         else { self.piece_bitboards[Piece::BlackPawn as usize] | self.piece_bitboards[Piece::BlackKnight as usize] | self.piece_bitboards[Piece::BlackBishop as usize] | self.piece_bitboards[Piece::BlackRook as usize] | self.piece_bitboards[Piece::BlackQueen as usize] | self.piece_bitboards[Piece::BlackKing as usize] };
        
        let all = us_bb | them_bb;

        let our_king_sq = if us == Player::White { first_bit(self.piece_bitboards[Piece::WhiteKing as usize]) } else { first_bit(self.piece_bitboards[Piece::BlackKing as usize]) };
        let their_king_sq = if us == Player::White { first_bit(self.piece_bitboards[Piece::BlackKing as usize]) } else { first_bit(self.piece_bitboards[Piece::WhiteKing as usize]) };

        let our_diagonal_sliders = self.diagonal_sliders(us);
        let our_orthogonal_sliders = self.orthogonal_sliders(us);
        let their_diagonal_sliders = self.diagonal_sliders(them);
        let their_orthogonal_sliders = self.orthogonal_sliders(them);

        let our_pawns_bb = if us == Player::White { self.piece_bitboards[Piece::WhitePawn as usize] } else { self.piece_bitboards[Piece::BlackPawn as usize] };
        let our_knights_bb = if us == Player::White { self.piece_bitboards[Piece::WhiteKnight as usize] } else { self.piece_bitboards[Piece::BlackKnight as usize] };

        let their_pawns_bb = if us == Player::White { self.piece_bitboards[Piece::BlackPawn as usize] } else { self.piece_bitboards[Piece::WhitePawn as usize] };
        let their_knights_bb = if us == Player::White { self.piece_bitboards[Piece::BlackKnight as usize] } else { self.piece_bitboards[Piece::WhiteKnight as usize] };

        // general purpose bitboards for attacks, masks, etc. 
        let mut bb1 : u64 = 0;
        let mut bb2 : u64 = 0;
        let mut bb3 : u64 = 0;

        // squares the king can not move to
        let mut danger :u64 = 0;


        /***************************************************************************/
        /*                                                                         */
        /* CALCULATION OF ATTACKMAP                                                */
        /*                                                                         */
        /* The attack map is a bitboard which contains all squares attacked by the */
        /* enemy. It is used to determine if a move is legal or not (i.e. if the   */
        /* king is in check).                                                      */
        /*                                                                         */
        /***************************************************************************/

        // FOR EACH enemy piece, add it's attacks to the danger bitboard
        // attacks by enenmy PAWNS and KING
        danger |= attacks_pawn_multiple(their_pawns_bb, them) | KING_ATTACKS[their_king_sq as usize];
        
        // atacks by enemy KNIGHTS
        bb1 = their_knights_bb;
        while bb1 != 0 {
            let knight_sq = pop_first_bit(&mut bb1);
            danger |= KNIGHT_ATTACKS[knight_sq as usize];
        }

        // diagonal attacks by enemy BISHOPS and QUEENS
        // NOTICE: we xor out the our king square to secure that squares x-rayed are also included in attack mask
        bb1 = their_diagonal_sliders;
        while bb1 != 0 {
            let slider_sq = pop_first_bit(&mut bb1);
            danger |= attacks_bishop(slider_sq, all ^ SQUARE_BB[our_king_sq as usize]);
        }

        // orthogonal attacks by enemy ROOKS and QUEENS
        // NOTICE: we xor out the our king square to secure that squares x-rayed are also included in attack mask
        bb1 = their_orthogonal_sliders;
        while bb1 != 0 {
            let slider_sq = pop_first_bit(&mut bb1);
            danger |= attacks_rook(slider_sq, all ^ SQUARE_BB[our_king_sq as usize]);
        }

        /***************************************************************************/
        /*                                                                         */
        /* CALCULATION OF KING MOVES                                               */
        /*                                                                         */
        /* The king moves are calculated first, since they are the easiest to      */
        /* calculate. One only needs to know if the king moves to a square which   */
        /* is attacked by the enemy.                                               */
        /*                                                                         */
        /***************************************************************************/

        // king can move to surroding sqaures, except attacked squares and squares occupied by own pieces
        bb1 = KING_ATTACKS[our_king_sq as usize] & !(us_bb | danger);
        make_moves_quiet(&mut self.moves, our_king_sq, bb1 & !them_bb);
        make_moves_capture(&mut self.moves, our_king_sq, bb1 & them_bb);

        // save danger map (for possible use in eval)
        self.attackmap = danger;


        /***************************************************************************/
        /*                                                                         */
        /* CALCULATION OF CHECKING AND PINNED PIECES                               */
        /*                                                                         */
        /* The checking and pinned pieces are calculated next, since they are      */
        /* needed to calculate the moves of other pieces.                          */
        /* The idea is to calculate a capture and a quiet mask. These masks limit  */
        /* the possible moves of the pieces. See next steps for details.           */
        /*                                                                         */
        /***************************************************************************/

        // the capture mask filters destination squares to those occupied by enemy pieces which are checking
        // the king and must be captured
        let capture_mask : u64;

        // the quiet mask filters destination squares to those where pieces must be moved to, to block
        // an incoming attack on the king (i.e. a check)
        let quiet_mask : u64;

        // a general purpose square for storing destination, etc.
        let mut sq : u8 = 0;

        // checking pieces are identified by:
        //   1. computing the (reverse) attacks from the own king square
        //   2. intersecting the attacks (the attack bitboard) with the enemy pieces (enemy bitboard)
        self.checkers = (KNIGHT_ATTACKS[our_king_sq as usize] & their_knights_bb) | (attacks_pawn_single(our_king_sq, us) & their_pawns_bb);


        // 1. compute sliding attacks from king square (excluding our pieces from the blockers mask, to find potential pins in the next step).
        // 2. find bitboard of checking/pinning sliding pieces by intersecting with board in point 1.
        let mut candidates = (attacks_rook(our_king_sq, them_bb) & their_orthogonal_sliders) | (attacks_bishop(our_king_sq, them_bb) & their_diagonal_sliders);

        // the next bit of code determines checking and pinned pieces
        self.pinned = 0;
        while candidates != 0 {
            // sqaure of the sliding piece
            sq = pop_first_bit(&mut candidates);
            
            // bitboard of pinned pieces between the piece on sq and our king
            bb1 = SQUARES_BETWEEN_BB[our_king_sq as usize][sq as usize] & us_bb;
            
            // if there are no pieces between the sliding piece and our king, the sliding piece is checking
            if bb1 == 0 {
                self.checkers |= SQUARE_BB[sq as usize];
            } 
            // else if there is only one piece between the sliding piece and our king, our piece is pinned
            else if (bb1 & (bb1 - 1)) == 0 {
                self.pinned |= bb1;
            }
        }

        // here CHECKERS and PINNED have been calculated


        /***************************************************************************/
        /*                                                                         */
        /* CALCULATION OF MOVES : PART I                                           */
        /*                                                                         */
        /* In the following, we calculate (a part of all) moves, depending on      */
        /* whether the king is in double check, single check or not in check at    */
        /* all.                                                                    */
        /* Double and single check are special cases, which can be handled         */
        /* more efficiently since they limit the number of possible moves.         */
        /*                                                                         */
        /* (1) Double Check                                                        */
        /*     -> we can return early, since only king moves are legal (which      */
        /*        have already been calculated)                                    */
        /* (2) Single Check                                                        */
        /*     -> we can return early, if the checking piece is a pawn or knight   */
        /*        (we only need to consider captures of the checking piece)        */
        /*     -> otherwise, we calculate the capture mask and the quiet mask      */
        /*        (and calculate the remaining moves in the next step)             */
        /*        (NOTE that only non-pinned pieces will be able to move i.e.      */
        /*         resolve the check, since they would expose the king to check    */
        /*         by a different piece if they moved)                             */       
        /* (3) Not in Check                                                        */
        /*     -> we handle a special en passant case                              */
        /*     -> we determine all possible castling moves                         */
        /*     -> we determine all possible moves for pinned pieces                */
        /*        (NOTE in contrast to above, pinned pieces can move, since they   */
        /*        can move along the pinning line because there is no check        */
        /*        to be resolved)                                                  */
        /*     -> we calculate the capture mask and the quiet mask                 */
        /*        (and calculate the remaining moves in the next step)             */
        /*                                                                         */
        /***************************************************************************/

        let not_pinned = !self.pinned;

        match sparse_pop_count(self.checkers){
            // DOUBLE CHECK
            2 => {
                // we can return here, since only king moves are legal
                return;
            },
            // SINGLE CHECK
            1 => {
                // we have already computed king moves, hence we only need to consider other pieces which either
                // (1) capture the checking piece 
                // (2) + for sliding checkers, moving the piece between the checking piece and our king (a.k.a blocking the line of threat)
                
                let checker_square = first_bit(self.checkers);
                let checker_piece = match self.playingfield[checker_square as usize] {
                    Square::Occupied(piece) => piece,
                    _ => panic!("Checker square is empty")
                };

                match checker_piece {
                    Piece::WhitePawn | Piece::BlackPawn | Piece::WhiteKnight | Piece::BlackKnight => {
                        // for pawns and knights, the only way to get out of check is to capture the checking piece
                        
                        // special case: if the checking piece is a pawn, we also have to consider en passant captures
                        if checker_piece.is_pawn() && self.checkers == shift(SQUARE_BB[self.history[self.ply as usize].epsq as usize], relative_dir(us, Direction::South)) {
                            bb1 = attacks_pawn_single(self.history[self.ply as usize].epsq, them) & our_pawns_bb & not_pinned;

                            while bb1 != 0 {
                                let from = pop_first_bit(&mut bb1);
                                self.moves.push(Move::new(from, self.history[self.ply as usize].epsq, MoveFlags::EnPassant));
                            }
                        }
                        // special case handled!
                        
                        // continue with normal captures
                        bb1 = self.attackers_from(checker_square, all, us) & not_pinned;
                        while bb1 != 0 {
                            let from = pop_first_bit(&mut bb1);
                            self.moves.push(Move::new(from, checker_square, MoveFlags::Capture));
                        }
                        return;
                    },
                    _ => {
                        // for all the other pieces, we can either 
                        // (1) capture the checking piece
                        capture_mask = self.checkers;
                        self.capture_mask = capture_mask;
                        // (2) move a piece between the checking piece and our king
                        quiet_mask = SQUARES_BETWEEN_BB[our_king_sq as usize][checker_square as usize];
                        self.quiet_mask = quiet_mask;
                    }
                }
            },
            // NOT IN CHECK
            _ => {
                // we can capture any enemy piece
                capture_mask = them_bb;
                self.capture_mask = capture_mask;

                // we can move (quiet move) any piece to any empty square
                quiet_mask = !all;
                self.quiet_mask = quiet_mask;

                // SPECIAL HANDLING of possible EP CAPTURES

                // check if there is a possible ep square to capture on
                if self.history[self.ply as usize].epsq != NO_SQUARE {
                    // if yes, compute bitboard of pawns which could capture on ep square
                    bb2 = attacks_pawn_single(self.history[self.ply as usize].epsq, them) & our_pawns_bb;

                    // (1) FIRST, we look at UNPINNED pawns which can capture on ep square
                    bb1 = bb2 & not_pinned;

                    while bb1 != 0 {
                        let from = pop_first_bit(&mut bb1);

                        // unfortunately, we have to do a second check here due to the infamous 'pseudo-pinned' e.p case
                        //
                        // - - - - - - - -
                        // - - - - - - - -
                        // - - - - - - - -
                        // k - - P p - - R
                        // - - - - - - - -
                        // - - - - - - - -
                        // - - - K - - - -
                        //
                        // the pawn on e4 is not directly pinned, but it can not capture on d3, since it would expose the king 
                        // to check by the rook on h4. 

                        // we XOR out the capturing piece and the 'ep-pawn', then check if the king would be attacked on the 
                        // line by a sliding piece (i.e. rook or queen)
                        let occ = all ^ SQUARE_BB[from as usize] ^ shift(SQUARE_BB[self.history[self.ply as usize].epsq as usize], relative_dir(us, Direction::South));

                        if (attacks_on_line(our_king_sq, occ, MASK_RANK[rank_of(our_king_sq) as usize]) & their_orthogonal_sliders) == 0 {
                            self.moves.push(Move::new(from, self.history[self.ply as usize].epsq, MoveFlags::EnPassant));
                        }

                        // WARNING: The same situation for diagonal attacks (see "8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1") is not handled
                        // In a normal game of chess, this situation can not occur, since the king would have been in check before
                        // the pawn made a doubkle push (in this case to d4).
                        // However, some chess engines like Stockfish do handle this case
                        // !!! simply be aware that move generation on custom made positions might not be correct !!!
                    }

                    // (2) THEN, we look at PINNED pawns which can capture on ep square
                    // pinned pawns can only capture on ep square if they are pinned diagonally
                    // i.e. if the ep square is on the same diagonal as the king and the pawn
                    bb1 = bb2 & self.pinned & LINE_SPANNED_BB[our_king_sq as usize][self.history[self.ply as usize].epsq as usize];
                    if bb1 != 0{
                        self.moves.push(Move::new(first_bit(bb1), self.history[self.ply as usize].epsq, MoveFlags::EnPassant));
                    }
                }

                // SPECIAL HANDLING of possible CASTLING moves

                // casteling is possible, if
                // (1) the king is not in check, moving through check or lands in check
                // (2) the king and the corresponding rook have not moved yet
                // (3) the squares between the king and the rook are empty

                // SHORTSIDE CASTLE
                if ((all | danger) & oo_mask(us)) == 0 && oo_allowed(us, self.history[self.ply as usize].castle) {
                    match us {
                        Player::White => self.moves.push(Move::new(4, 6, MoveFlags::KingCastle)),
                        Player::Black => self.moves.push(Move::new(60, 62, MoveFlags::KingCastle))
                    }
                }

                // LONGSIDE CASTLE
                // NOTE, since attacks on the b square are not relevant for casteling,
                // we have to mask it out when calculating casterling moves
                if ((all | (danger & !ooo_ignore_b_file(us))) & ooo_mask(us)) == 0 &&   ooo_allowed(us, self.history[self.ply as usize].castle) {
                    match us {
                        Player::White => self.moves.push(Move::new(4, 2, MoveFlags::QueenCastle)),
                        Player::Black => self.moves.push(Move::new(60, 58, MoveFlags::QueenCastle))
                    }
                }

                // SPECIAL HANDLING of PINNED PIECE moves

                // pinned BISHOPS, ROOKS, QUEENS
                bb1 = !(not_pinned | our_pawns_bb | our_knights_bb);
                while bb1 != 0 {
                    let from = pop_first_bit(&mut bb1);
                    let piece = match self.playingfield[from as usize] {
                        Square::Occupied(piece) => piece,
                        _ => panic!("Pinned piece square is empty")
                    };

                    match piece {
                        Piece::WhiteBishop | Piece::BlackBishop => {
                            bb2 = attacks_bishop(from, all) & LINE_SPANNED_BB[from as usize][our_king_sq as usize];
                        },
                        Piece::WhiteRook | Piece::BlackRook => {
                            bb2 = attacks_rook(from, all) & LINE_SPANNED_BB[from as usize][our_king_sq as usize];
                        },
                        Piece::WhiteQueen | Piece::BlackQueen => {
                            bb2 = (attacks_bishop(from, all) | attacks_rook(from, all)) & LINE_SPANNED_BB[from as usize][our_king_sq as usize];
                        },
                        _ => panic!("Pinned piece is not a sliding piece")
                    }

                    make_moves_quiet(&mut self.moves, from, bb2 & quiet_mask);
                    make_moves_capture(&mut self.moves, from, bb2 & capture_mask);
                }

                // pinned PAWNS
                bb1 = !not_pinned & our_pawns_bb;

                while bb1 != 0{
                    let from = pop_first_bit(&mut bb1);

                    if (rank_of(from) as i8) == (relative_rank(us, Rank::Seven) as i8){
                        bb2 = attacks_pawn_single(from, us) & capture_mask & LINE_SPANNED_BB[from as usize][our_king_sq as usize];
                        make_moves_promcapture(&mut self.moves, from, bb2);
                        // NOTE, that quiet promotions are not possible, since either the pawn is pinned on the file
                        // (i.e. the pawn is blocked by the king or the pinner) or the pawn is pinned
                        // in such a way that promoting it would expose the king to check
                    } else {
                        // captures
                        bb2 = attacks_pawn_single(from, us) & capture_mask & LINE_SPANNED_BB[from as usize][our_king_sq as usize];
                        make_moves_capture(&mut self.moves, from, bb2);

                        // single pawn pushes
                        bb2 = shift(SQUARE_BB[from as usize], relative_dir(us, Direction::North)) & !all & LINE_SPANNED_BB[from as usize][our_king_sq as usize];

                        // double pawn pushes
                        bb3 = shift(bb2 & MASK_RANK[relative_rank(us, Rank::Three) as usize], relative_dir(us, Direction::North)) & !all & LINE_SPANNED_BB[from as usize][our_king_sq as usize];

                        make_moves_quiet(&mut self.moves, from, bb2);
                        make_moves_doublepush(&mut self.moves, from, bb3);
                    }
                }

                // pinned KNIGHTS (cannot move, so were done here)
            }
        }

        /***************************************************************************/
        /*                                                                         */
        /* CALCULATION OF MOVES : PART II                                          */
        /*                                                                         */
        /* In the following, we calculate the remaining moves, which are           */
        /* (1) non-pinned piece moves                                              */
        /* (2) pawn moves (including promotion)                                    */
        /*                                                                         */
        /* The capture and quiet masks are taken into consideration. In the case   */
        /* of single check, the capture mask is set to the checking piece, and the */
        /* quiet mask is set to the squares between the checking piece and our     */
        /* king (i.e. the line of threat which one might be able to block).        */
        /*                                                                         */
        /* In the case of no check, the capture mask is set to all enemy pieces    */
        /* and the quiet mask is set to all empty squares.                         */
        /*                                                                         */
        /***************************************************************************/

        // non-pinned KNIGHT moves
        bb1 = our_knights_bb & not_pinned;
        while bb1 != 0 {
            let from = pop_first_bit(&mut bb1);
            bb2 = KNIGHT_ATTACKS[from as usize];
            make_moves_quiet(&mut self.moves, from, bb2 & quiet_mask);
            make_moves_capture(&mut self.moves, from, bb2 & capture_mask);
        }

        // non-pinned BISHOP and QUEEN (diagonal) moves
        bb1 = our_diagonal_sliders & not_pinned;
        while bb1 != 0 {
            let from = pop_first_bit(&mut bb1);
            bb2 = attacks_bishop(from, all);
            make_moves_quiet(&mut self.moves, from, bb2 & quiet_mask);
            make_moves_capture(&mut self.moves, from, bb2 & capture_mask);
        }

        // non-pinned ROOK and QUEEN (orthogonal) moves
        bb1 = our_orthogonal_sliders & not_pinned;
        while bb1 != 0 {
            let from = pop_first_bit(&mut bb1);
            bb2 = attacks_rook(from, all);
            make_moves_quiet(&mut self.moves, from, bb2 & quiet_mask);
            make_moves_capture(&mut self.moves, from, bb2 & capture_mask);
        }

        // determine pawns which are NOT about to promote
        bb1 = our_pawns_bb & not_pinned & !MASK_RANK[relative_rank(us, Rank::Seven) as usize];

        // SINGLE PAWN PUSHES
        bb2 = shift(bb1, relative_dir(us, Direction::North)) & !all;

        // DOUBLE PAWN PUSHES (only pawns on 3/6 from bb2 are eligible)
        bb3 = shift(bb2 & MASK_RANK[relative_rank(us, Rank::Three) as usize], relative_dir(us, Direction::North)) & quiet_mask;

        // we AND(&) bb2 with the quiet mask only now (and not before bb3), as a non-check-blocking single push (a.k.a single pish & quiet_mask)
        // does not mean that the corresponding double push is also non-check-blocking
        bb2 &= quiet_mask;

        while bb2 != 0 {
            let to = pop_first_bit(&mut bb2);
            self.moves.push(Move::new(((to as i8) - (relative_dir(us, Direction::North) as i8)) as u8, to, MoveFlags::Quiet));
        }

        while bb3 != 0 {
            let to = pop_first_bit(&mut bb3);
            self.moves.push(Move::new(((to as i8) - (relative_dir(us, Direction::NorthNorth) as i8)) as u8, to, MoveFlags::DoublePawnPush));
        }

        // PAWN CAPTURES
        bb2 = shift(bb1, relative_dir(us, Direction::NorthWest)) & capture_mask;
        bb3 = shift(bb1, relative_dir(us, Direction::NorthEast)) & capture_mask;

        while bb2 != 0 {
            let to = pop_first_bit(&mut bb2);
            self.moves.push(Move::new(((to as i8) - (relative_dir(us, Direction::NorthWest) as i8)) as u8, to, MoveFlags::Capture));
        }

        while bb3 != 0 {
            let to = pop_first_bit(&mut bb3);
            self.moves.push(Move::new(((to as i8) - (relative_dir(us, Direction::NorthEast) as i8)) as u8, to, MoveFlags::Capture));
        }

        // PROMOTIONS

        // determine pawns which are about to promote
        bb1 = our_pawns_bb & not_pinned & MASK_RANK[relative_rank(us, Rank::Seven) as usize];
        if bb1 != 0 {
            // QUIET PROMOTIONS
            bb2 = shift(bb1, relative_dir(us, Direction::North)) & quiet_mask;
            while bb2 != 0 {
                let to = pop_first_bit(&mut bb2);
                let from = ((to as i8) - (relative_dir(us, Direction::North) as i8)) as u8;
                self.moves.push(Move::new(from, to, MoveFlags::KnightPromo));
                self.moves.push(Move::new(from, to, MoveFlags::BishopPromo));
                self.moves.push(Move::new(from, to, MoveFlags::RookPromo));
                self.moves.push(Move::new(from, to, MoveFlags::QueenPromo));
            }

            // CAPTURE PROMOTIONS
            bb2 = shift(bb1, relative_dir(us, Direction::NorthWest)) & capture_mask;
            bb3 = shift(bb1, relative_dir(us, Direction::NorthEast)) & capture_mask;

            while bb2 != 0 {
                let to = pop_first_bit(&mut bb2);
                let from = ((to as i8) - (relative_dir(us, Direction::NorthWest) as i8)) as u8;
                self.moves.push(Move::new(from, to, MoveFlags::KnightPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::BishopPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::RookPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::QueenPromoCapture));
            }

            while bb3 != 0 {
                let to = pop_first_bit(&mut bb3);
                let from = ((to as i8) - (relative_dir(us, Direction::NorthEast) as i8)) as u8;
                self.moves.push(Move::new(from, to, MoveFlags::KnightPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::BishopPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::RookPromoCapture));
                self.moves.push(Move::new(from, to, MoveFlags::QueenPromoCapture));
            }
        }

        // YEAH! We are done!
    }
}


const fn ooo_ignore_b_file(player : Player) -> u64 {
    match player {
        Player::White => 0x2,
        Player::Black => 0x200000000000000
    }
}

const fn oo_mask(player : Player) -> u64 {
    match player {
        Player::White => 0x60,
        Player::Black => 0x6000000000000000
    }
}

const fn oo_allowed(player : Player, castlerights : u8) -> bool {
    match player {
        Player::White => castlerights & CASTLE_WHITE_KING != 0,
        Player::Black => castlerights & CASTLE_BLACK_KING != 0
    }
}

const fn ooo_mask(player : Player) -> u64 {
    match player {
        Player::White => 0xe,
        Player::Black => 0xe00000000000000
    }
}

const fn ooo_allowed(player : Player, castlerights : u8) -> bool {
    match player {
        Player::White => castlerights & CASTLE_WHITE_QUEEN != 0,
        Player::Black => castlerights & CASTLE_BLACK_QUEEN != 0
    }
}

fn relative_dir(player : Player, dir : Direction) -> Direction {
    match player{
        Player::White => dir,
        Player::Black => match dir {
            Direction::North => Direction::South,
            Direction::NorthNorth => Direction::SouthSouth,
            Direction::South => Direction::North,
            Direction::SouthSouth => Direction::NorthNorth,
            Direction::East => Direction::West,
            Direction::West => Direction::East,
            Direction::NorthEast => Direction::SouthWest,
            Direction::NorthWest => Direction::SouthEast,
            Direction::SouthEast => Direction::NorthWest,
            Direction::SouthWest => Direction::NorthEast
        }
    }
}

fn relative_rank(player : Player, rank : Rank) -> Rank {
    match player {
        Player::White => rank,
        Player::Black => match rank {
            Rank::One => Rank::Eight,
            Rank::Two => Rank::Seven,
            Rank::Three => Rank::Six,
            Rank::Four => Rank::Five,
            Rank::Five => Rank::Four,
            Rank::Six => Rank::Three,
            Rank::Seven => Rank::Two,
            Rank::Eight => Rank::One
        }
    }
}

fn shift(board: u64, dir : Direction) -> u64 {
    match dir {
        Direction::North => board << 8,
        Direction::NorthNorth => board << 16,
        Direction::South => board >> 8,
        Direction::SouthSouth => board >> 16,
        Direction::East => (board & CLEAR_FILE[File::H as usize]) << 1,
        Direction::West => (board & CLEAR_FILE[File::A as usize]) >> 1,
        Direction::NorthEast => (board & CLEAR_FILE[File::H as usize]) << 9,
        Direction::NorthWest => (board & CLEAR_FILE[File::A as usize]) << 7,
        Direction::SouthEast => (board & CLEAR_FILE[File::H as usize]) >> 7,
        Direction::SouthWest => (board & CLEAR_FILE[File::A as usize]) >> 9
    }
} 

fn attacks_pawn_multiple(pawns : u64, player : Player) -> u64 {
    let mut attacks = 0;

    if player == Player::White {
        attacks |= shift(pawns, Direction::NorthEast) | shift(pawns, Direction::NorthWest);
    } else {
        attacks |= shift(pawns, Direction::SouthEast) | shift(pawns, Direction::SouthWest);
    }

    attacks
}

fn attacks_pawn_single(sq : u8, player : Player) -> u64 {
    PAWN_ATTACKS[player as usize][sq as usize]
}

fn attacks_bishop(sq : u8, occ : u64) -> u64 {
    let j = transform(occ & BISHOP_MASK[sq as usize], BISHOP_MAGIC[sq as usize], BISHOP_BITS[sq as usize] as i32);
    BISHOP_ATTACKS_BLOCKERS_CONSIDERED[sq as usize][j as usize]
}

fn attacks_rook(sq : u8, occ : u64) -> u64 {
    let j = transform(occ & ROOK_MASK[sq as usize], ROOK_MAGIC[sq as usize], ROOK_BITS[sq as usize] as i32);
    ROOK_ATTACKS_BLOCKERS_CONSIDERED[sq as usize][j as usize]
}

fn make_moves_quiet(moves : &mut Vec<Move>, from : u8, to : u64) {
    let mut bb = to;
    while bb != 0 {
        let to_sq = pop_first_bit(&mut bb);
        moves.push(Move::new(from, to_sq, MoveFlags::Quiet));
    }
}

fn make_moves_capture(moves : &mut Vec<Move>, from : u8, to : u64) {
    let mut bb = to;
    while bb != 0 {
        let to_sq = pop_first_bit(&mut bb);
        moves.push(Move::new(from, to_sq, MoveFlags::Capture));
    }
}

fn make_moves_promcapture(moves : &mut Vec<Move>, from : u8, to : u64) {
    let mut bb = to;
    while bb != 0 {
        let to_sq = pop_first_bit(&mut bb);
        moves.push(Move::new(from, to_sq, MoveFlags::KnightPromoCapture));
        moves.push(Move::new(from, to_sq, MoveFlags::BishopPromoCapture));
        moves.push(Move::new(from, to_sq, MoveFlags::RookPromoCapture));
        moves.push(Move::new(from, to_sq, MoveFlags::QueenPromoCapture));
    }
}

fn make_moves_doublepush(moves : &mut Vec<Move>, from : u8, to : u64) {
    let mut bb = to;
    while bb != 0 {
        let to_sq = pop_first_bit(&mut bb);
        moves.push(Move::new(from, to_sq, MoveFlags::DoublePawnPush));
    }
}

const fn first_bit(bb : u64) -> u8 {
    const DEBRUIJN_MAGIC : u64 = 0x03f79d71b4cb0a89;
    let num = bb ^ (bb - 1);
    DEBRUIJN_TABLE[(num.wrapping_mul(DEBRUIJN_MAGIC) >> 58) as usize]
}

fn pop_first_bit(bb : &mut u64) -> u8 {
    if *bb == 0 {
        panic!("pop_first_bit called on empty bitboard");
    }

    let bit = first_bit(*bb);
    *bb &= *bb - 1;
    bit
}

fn sparse_pop_count(mut bb : u64) -> u8 {
    let mut count = 0;
    while bb != 0 {
        count += 1;
        bb &= bb.wrapping_sub(1);
    }
    count
}

const fn initialize_mask_file_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut file = 0;
    while file < 8 {
        table[file] = 0x101010101010101 << file;
        file += 1;
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

const fn initialize_mask_rank_table() -> [u64; 8] {
    let mut table = [0; 8];

    let mut rank = 0;
    while rank < 8 {
        table[rank] = 0xFF << (rank * 8);
        rank += 1;
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

const fn transform(mask : u64, magic : u64, bits : i32) -> i32 {
    /* depending on the blockers mask, we need a different key to access the
     * correct attack map */
    (mask.wrapping_mul(magic)  >> (64 - bits)) as i32
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

const fn initialize_bishop_attack_table_blockers_considered() -> [[u64; 4096]; 64] {
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

const fn attacks_on_line(sq : u8, occ : u64, mask : u64) -> u64 {
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

const fn rank_of(sq : u8) -> u8 {
    sq >> 3
}

const fn file_of(sq : u8) -> u8 {
    sq & 7
}

const fn diagonal_of(sq : u8) -> u8 {
    rank_of(sq) + 7 - file_of(sq)
}

const fn anti_diagonal_of(sq : u8) -> u8 {
    rank_of(sq) + file_of(sq)
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

fn print_bitboard(bb : &u64) {
    println!();
    for rank in (0..8).rev() {
        for file in 0..8 {  
            if (bb & (1 << (rank * 8 + file))) != 0 {
                print!("O ");
            } else {
                print!("- ");
            }
        }
        println!();
    }
    println!("\n");
}

fn perft(board : &mut Board, depth : u8) -> u64 {
    if depth == 0 {
        return 1;
    }

    let mut nodes = 0;
    
    board.generate_moves();

    let moves = board.moves.clone();

    for m in moves {
        board.do_move(m);

        nodes += perft(board, depth - 1);

        board.undo_move(m);
    }

    nodes
}

fn perft_divide(board : &mut Board, depth : u8) -> u64 {
    board.generate_moves();

    let moves = board.moves.clone();

    let mut nodes = 0;

    for m in moves {
        //board.print();
        board.do_move(m);

        let n = perft(board, depth - 1);
        nodes += n;
        println!("{:?}: {}", m, n);
        board.undo_move(m);
    }

    nodes
}

use std::path::Path;
use std::io::{BufRead, BufReader};

fn run_perft_tests(filename : &str) {
    let path = Path::new(filename);

    let file = std::fs::File::open(path).unwrap();
    let reader = BufReader::new(file);

    println!("+--------------------------------------------------------------------------------------+--------+------------+------------+-------------------+----------+");
    println!("| FEN                                                                                  | Depth  | Expected   | Result     | MNodes per second | Correct  |");
    println!("+--------------------------------------------------------------------------------------+--------+------------+------------+-------------------+----------+");

    let mut global_nodes = 0;
    let global_start = std::time::Instant::now();
    let mut global_fail_count = 0;

    for line in reader.lines() {
        let line = line.unwrap();
        let mut parts = line.split(';');
        let fen = parts.next().unwrap();
        
        // iterate over all depths
        for part in parts {
            let mut depth_result = part.split(' ');
            let depth = depth_result.next().unwrap();
            let depth = depth[1..].parse::<u8>().unwrap();
            let nodes = depth_result.next().unwrap().parse::<u64>().unwrap();

            let mut board = Board::new();
            board.set_by_fen(fen);

            let start = std::time::Instant::now();
            let result = perft(&mut board, depth);
            global_nodes += result;
            let duration = start.elapsed();

            let duration_in_ms = duration.as_secs() * 1000 + duration.subsec_millis() as u64;

            let correct = if result == nodes { "\x1b[0;32m yes \x1b[0m" } else { global_fail_count += 1; "\x1b[0;31m no \x1b[0m" };


            let mnps;
            if duration_in_ms == 0 {
                mnps = "inf".to_string();
            } else {
                mnps = format!("{:.*}", 4, result as f64 / (duration_in_ms as f64 / 1000.0) / 1000000.0);
            }
            println!("| {:<84} | {:<6} | {:<10} | {:<10} | {:<17} |  {:<8}   |", fen, depth, nodes, result, mnps, correct);
            println!("+--------------------------------------------------------------------------------------+--------+------------+------------+-------------------+----------+");

        }
    }
    let global_duration = global_start.elapsed();
    let global_duration_in_ms = global_duration.as_secs() * 1000 + global_duration.subsec_millis() as u64;
    let global_mnps;
    if global_duration_in_ms == 0 {
        global_mnps = "inf".to_string();
    } else {
        global_mnps = format!("{:.*}", 4, global_nodes as f64 / (global_duration_in_ms as f64 / 1000.0) / 1000000.0);
    }
    
    if global_fail_count == 0 {
        print!("\n\x1b[0;32m All tests passed! \x1b[0m\n {} nodes in {} ms ({} MNodes/s)\n", global_nodes, global_duration_in_ms, global_mnps);
    } else {
        print!("\n\x1b[0;31m {} tests failed! \x1b[0m\n {} nodes in {} ms ({} MNodes/s)\n", global_fail_count, global_nodes, global_duration_in_ms, global_mnps);
    }
}

fn main() {
    run_perft_tests("/Users/aherbrich/src/myprojects/engine/src/test_suite.txt");
}

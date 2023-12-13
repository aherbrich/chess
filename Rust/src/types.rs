use core::fmt;

#[derive(Copy, Clone, Eq, PartialEq, Debug)]
pub enum Player {
    White,
    Black
}

pub enum Direction {
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

pub enum File {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H
}

pub enum Rank {
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight
}

#[derive(Copy, Clone)]
pub enum Square {
    Empty,
    Occupied(Piece)
}

#[derive(Copy, Clone)]
pub enum Piece {
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
    pub fn is_pawn(&self) -> bool {
        match self {
            Piece::WhitePawn | Piece::BlackPawn => true,
            _ => false
        }
    }
}



/******************************************************/
/*                                                    */
/*  Move                                              */
/*                                                    */
/******************************************************/

pub enum MoveFlags {
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
    pub fn from_u8(val : u8) -> MoveFlags {
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

#[derive(Copy, Clone)]
pub struct Move {
    pub from : u8,
    pub to : u8,
    pub flags : u8
}

impl Move {
    pub fn new(from : u8, to : u8, flags : MoveFlags) -> Move {
        Move {
            from,
            to,
            flags : flags as u8
        }
    }
}

impl fmt::Debug for Move {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut result = String::new();

        const FIELD : [&str; 64] = ["a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2",
        "c2", "d2", "e2", "f2", "g2", "h2", "a3", "b3", "c3", "d3",
        "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4",
        "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7",
        "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8",
        "e8", "f8", "g8", "h8"];

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



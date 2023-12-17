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

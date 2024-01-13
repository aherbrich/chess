#![allow(non_snake_case)]
pub mod engine {
    pub mod board;
    pub mod perft;
    mod tables;
    mod helpers;
    mod types;
    mod moves;
    mod zobrist;
}

pub mod ordering {
    pub mod gaussian;
    pub mod factorgraph;
}
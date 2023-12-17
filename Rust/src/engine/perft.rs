use super::board::Board;

pub fn perft(board : &mut Board, depth : u8) -> u64 {
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

// fn perft_divide(board : &mut Board, depth : u8) -> u64 {
//     board.generate_moves();

//     let moves = board.moves.clone();

//     let mut nodes = 0;

//     for m in moves {
//         board.do_move(m);

//         let n = perft(board, depth - 1);
//         nodes += n;
//         println!("{:?}: {}", m, n);
//         board.undo_move(m);
//     }

//     nodes
// }
use std::path::Path;
use std::io::{BufRead, BufReader};

use TrueChess::engine::board::Board;
use TrueChess::engine::perft::perft;


#[test]
fn run_perft_tests() {
    let path = Path::new("/Users/aherbrich/src/myprojects/chess/Rust/tests/test_suite.txt");

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
        assert!(true);
    } else {
        print!("\n\x1b[0;31m {} tests failed! \x1b[0m\n {} nodes in {} ms ({} MNodes/s)\n", global_fail_count, global_nodes, global_duration_in_ms, global_mnps);
        assert!(false);
    }
}
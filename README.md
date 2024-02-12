

## Introduction

Welcome to my chess engine project! The engine (unofficially know as _Herby_) is a open source UCI compatible chess engine. 

## History

My journey into chess engine programming started during a semester break when I sought a coding project to immerse myself in. Initially, I didn't have a clear direction, but after stumbling upon Sebastian Lague's video [Coding Adventure: Chess](https://www.youtube.com/watch?v=U4ogK0MIzqk), I found my inspiration. From that moment, I was hooked.

Every semester break became an opportunity to further develop my chess engine. Even during the hectic academic periods, I'd dedicate any spare time to refining and improving my engine.

## Research and Development

Through the process of developing the chess engine, a desire emerged to explore the potential of extracting board patterns and learning from them. I'm particularly interested in creating an engine that mimics human-like thinking by considering only a few (1-2) moves deeply, solely based on patterns and "intuition".

For this purpose, I've also ventured into developing and experimenting with a new chess engine written in Julia, called [Poppy](https://github.com/aherbrich/Poppy). Julia offers a more flexible environment for research and experimentation compared to traditional languages like C, especially when it comes to testing and plotting.


## Rust Move Generation
Driven by the desire to learn Rust (Dec. 2023) and enhance the performance of my chess engine, I decided to rewrite the engine in Rust. Currently only the move generation part of the engine has been rewritten, but the plan is to eventually rewrite the entire engine.

The Rust implementation of the move generation algorithm now outperforms the C version, even surpassing industry-standard engines like [Stockfish](https://github.com/official-stockfish/Stockfish).

For example on the [Kiwipete position](https://www.chessprogramming.org/Perft_Results#Position_2), my Rust implementation achieves a speed of **275 million** nodes per second. In comparison, Stockfish reaches (only) **225 million** nodes per second on the same (my Apple M1) chip.

## Inspiration and Resources
- [chessprogramming.org](https://www.chessprogramming.org/) (Wiki)
- [Blunder](https://github.com/algerbrex/blunder) (Chess engine in Go)
- [surge](https://github.com/nkarve/surge) (Move generator in C++)

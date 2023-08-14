# Tool to test the C code of the Bayesian move ranking training
#
# 2023 written by Ralf Herbrich

# structure of a single chess game read from the file
struct ChessGame
    moveList::Cstring
    winner::Cint
end

# structure of all chess games read from the file
struct ChessGames
    no_games::Cint
    games::Ptr{Ptr{ChessGame}}
end
# structure for a single 1d gaussian_t
struct CGaussian
    τ::Cfloat
    ρ::Cfloat
end

# placeholder for a hash table of urgency beliefs
struct HTUrgencies end

# structure mirroring a training info
struct TrainingInfo
    ht_urgencies::Ptr{HTUrgencies}  # hash table of urgencies for each move (hash)
    prior::CGaussian                # prior for the urgencies 
    β::Cdouble                      # the standard deviation parameter for the latent urgencies
    fullTraining::Cint              # whether we are doing full training or incremental training 
    baseFilename::Cstring           # the base filename for storing snapshots of the model
    verbosity::Cint                 # the verbosity level
end

# runs a training run
function run()
    file_name = pwd() * "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn"
    chess_games = @ccall "lib/libchess.so".load_chess_games(file_name::Cstring)::ChessGames

    # initalizes the chess engine
    @ccall "lib/libchess.so".initialize_attack_boards()::Cvoid
    @ccall "lib/libchess.so".initialize_helper_boards()::Cvoid
    @ccall "lib/libchess.so".initialize_zobrist_table()::Cvoid
    @ccall "lib/libchess.so".initialize_ranking_updates()::Cvoid
    @ccall "lib/libchess.so".initialize_move_zobrist_table()::Cvoid

    # allocate a new hash table for urgencies
    urgencies = @ccall "lib/libchess.so".initialize_ht_urgencies()::Ptr{HTUrgencies}

    # train the model 
    train_info = TrainingInfo(urgencies, CGaussian(0, 1), 0.5, 0, C_NULL, 1)
    @ccall "lib/libchess.so".train_model(
        chess_games.games::Ptr{Ptr{ChessGame}},
        chess_games.no_games::Cint,
        train_info::TrainingInfo,
    )::Cvoid
    no_unique_moves =
        @ccall "lib/libchess.so".get_no_keys(urgencies::Ptr{HTUrgencies})::Cint
    println("Unique moves: $no_unique_moves")

    # write the urgencies to a file
    @ccall "lib/libchess.so".write_ht_urgencies_to_binary_file(
        "ht_julia_urgencies.bin"::Cstring,
        urgencies::Ptr{HTUrgencies},
    )::Cvoid

    # release the hash table for urgencies
    @ccall "lib/libchess.so".deletes_ht_urgencies(urgencies::Ptr{HTUrgencies})::Cvoid

    # release the games
    @ccall "lib/libchess.so".delete_chess_games(chess_games::ChessGames)::Cvoid
end

run()

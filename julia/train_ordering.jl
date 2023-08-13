# Tool to test the C code of the Bayesian move ranking training
#
# 2023 written by Ralf Herbrich

# structure of a single chess game read from the file
struct ChessGame
    moveList::Cstring
    winner::Cint
end

# structure for a single 1d gaussian_t
struct CGaussian
    τ::Cfloat
    ρ::Cfloat
end

# structure mirroring a training info
struct TrainingInfo
    ht_urgencies::Ptr{Cvoid}        # hash table of urgencies for each move (hash)
    prior::CGaussian                # prior for the urgencies 
    β::Cdouble                      # the standard deviation parameter for the latent urgencies
    fullTraining::Cint              # whether we are doing full training or incremental training 
    baseFilename::Cstring           # the base filename for storing snapshots of the model
    verbosity                       # the verbosity level
end

# runs a training run
function run()
    no_games = @ccall "lib/libchess.so".count_number_of_games()::Cint
    chess_games = @ccall "lib/libchess.so".parse_chessgames_file(no_games::Cint)::Ptr{Ptr{ChessGame}}

    # initalizes the chess engine
    @ccall "lib/libchess.so".initialize_attack_boards()::Cvoid
    @ccall "lib/libchess.so".initialize_helper_boards()::Cvoid
    @ccall "lib/libchess.so".initialize_zobrist_table()::Cvoid
    @ccall "lib/libchess.so".initialize_ranking_updates()::Cvoid
    @ccall "lib/libchess.so".initialize_move_zobrist_table()::Cvoid

    # allocate a new hash table for urgencies
    urgencies = @ccall "lib/libchess.so".initialize_ht_urgencies()::Ptr{Cvoid}

    # train the model 
    train_info = TrainingInfo(urgencies,CGaussian(0,1),0.5,0,C_NULL, 1)
    @ccall "lib/libchess.so".train_model(chess_games::Ptr{Ptr{ChessGame}}, no_games::Cint, train_info::TrainingInfo)::Cvoid
    no_unique_moves = @ccall "lib/libchess.so".get_no_keys(urgencies::Ptr{Cvoid})::Cint
    println("Unique moves: $no_unique_moves")

    # release the hash table for urgencies
    @ccall "lib/libchess.so".deletes_ht_urgencies(urgencies::Ptr{Cvoid})::Cvoid
end

run()


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
    τ::Cdouble
    ρ::Cdouble
end

# placeholder for a hash table of urgency beliefs
struct HTUrgencies end

# structure for a single urgency belief
struct HTUrgenciesListEntry
    move_key::Cint
    urgency::CGaussian
    next::Ptr{HTUrgenciesListEntry}
end

#structure for the iterator over the hash table of urgencies
mutable struct HTUrgenciesIterator
    cur_hash::Cint
    cur_urgency_entry::Ptr{HTUrgenciesListEntry}
end

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
function run(file_name)
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
    train_info = TrainingInfo(urgencies, CGaussian(0.0,1.0), 0.5, 0, C_NULL, 1)
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

    # # iterate over the urgencies
    # urgencies_iterator = HTUrgenciesIterator(0, C_NULL)
    # @ccall "lib/libchess.so".setup_ht_urgencies_iterator(urgencies::Ptr{HTUrgencies}, urgencies_iterator::Ref{HTUrgenciesIterator})::Cvoid
    # finished = 0
    # cnt = 1
    # while (finished == 0)
    #     urgency_entry = unsafe_load(urgencies_iterator.cur_urgency_entry)
    #     println("Move $(cnt): $(urgency_entry.move_key), urgency: $(urgency_entry.urgency.τ), $(urgency_entry.urgency.ρ)")
    #     @ccall "lib/libchess.so".inc_ht_urgencies_iterator(urgencies::Ptr{HTUrgencies}, urgencies_iterator::Ref{HTUrgenciesIterator})::Cvoid
    #     finished = @ccall "lib/libchess.so".ht_urgencies_iterator_finished(urgencies_iterator::Ref{HTUrgenciesIterator})::Cint
    #     cnt += 1
    # end

    # release the hash table for urgencies
    @ccall "lib/libchess.so".deletes_ht_urgencies(urgencies::Ptr{HTUrgencies})::Cvoid

    # release the games
    @ccall "lib/libchess.so".delete_chess_games(chess_games::ChessGames)::Cvoid
end

@time run(pwd() * "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn")
# run("/tmp/very_short_games.pgn")

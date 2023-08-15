# Tool to test the C code of the Bayesian move ranking training
#
# 2023 written by Ralf Herbrich

using Libdl

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
function run(lib, file_name)
    load_chess_games = dlsym(lib, :load_chess_games)
    initialize_attack_boards = dlsym(lib, :initialize_attack_boards)
    initialize_helper_boards = dlsym(lib, :initialize_helper_boards)
    initialize_zobrist_table = dlsym(lib, :initialize_zobrist_table)
    initialize_ranking_updates = dlsym(lib, :initialize_ranking_updates)
    initialize_move_zobrist_table = dlsym(lib, :initialize_move_zobrist_table)
    initialize_ht_urgencies = dlsym(lib, :initialize_ht_urgencies)
    train_model = dlsym(lib, :train_model)
    get_no_keys = dlsym(lib, :get_no_keys)
    write_ht_urgencies_to_binary_file = dlsym(lib, :write_ht_urgencies_to_binary_file)
    setup_ht_urgencies_iterator = dlsym(lib, :setup_ht_urgencies_iterator)
    inc_ht_urgencies_iterator = dlsym(lib, :inc_ht_urgencies_iterator)
    ht_urgencies_iterator_finished = dlsym(lib, :ht_urgencies_iterator_finished)
    deletes_ht_urgencies = dlsym(lib, :deletes_ht_urgencies)
    delete_chess_games = dlsym(lib, :delete_chess_games)

    # load the chess games from the file
    chess_games = @ccall $load_chess_games(file_name::Cstring)::ChessGames

    # initalizes the chess engine
    @ccall $initialize_attack_boards()::Cvoid
    @ccall $initialize_helper_boards()::Cvoid
    @ccall $initialize_zobrist_table()::Cvoid
    @ccall $initialize_ranking_updates()::Cvoid
    @ccall $initialize_move_zobrist_table()::Cvoid

    # allocate a new hash table for urgencies
    urgencies = @ccall $initialize_ht_urgencies()::Ptr{HTUrgencies}

    # train the model 
    train_info = TrainingInfo(urgencies, CGaussian(0.0, 1.0), 0.5, 0, C_NULL, 1)
    @ccall $train_model(
        chess_games.games::Ptr{Ptr{ChessGame}},
        chess_games.no_games::Cint,
        train_info::TrainingInfo,
    )::Cvoid
    no_unique_moves = @ccall $get_no_keys(urgencies::Ptr{HTUrgencies})::Cint
    println("Unique moves: $no_unique_moves")

    # write the urgencies to a file
    @ccall $write_ht_urgencies_to_binary_file(
        "ht_julia_urgencies.bin"::Cstring,
        urgencies::Ptr{HTUrgencies},
    )::Cvoid

    # # iterate over the urgencies
    # it = HTUrgenciesIterator(0, C_NULL)
    # @ccall $setup_ht_urgencies_iterator(urgencies::Ptr{HTUrgencies}, it::Ref{HTUrgenciesIterator})::Cvoid
    # finished = 0
    # cnt = 1
    # while (finished == 0)
    #     urgency_entry = unsafe_load(it.cur_urgency_entry)
    #     println("Move $(cnt): $(urgency_entry.move_key), urgency: $(urgency_entry.urgency.τ), $(urgency_entry.urgency.ρ)")
    #     @ccall $inc_ht_urgencies_iterator(urgencies::Ptr{HTUrgencies}, it::Ref{HTUrgenciesIterator})::Cvoid
    #     finished = @ccall $ht_urgencies_iterator_finished(it::Ref{HTUrgenciesIterator})::Cint
    #     cnt += 1
    # end

    # release the hash table for urgencies
    @ccall $deletes_ht_urgencies(urgencies::Ptr{HTUrgencies})::Cvoid

    # release the games
    @ccall $delete_chess_games(chess_games::ChessGames)::Cvoid
end

lib = dlopen("lib/libchess.so")
try
    @time run(lib, pwd() * "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn")
finally
    dlclose(lib)
end

# Tool to test the accuracy of Bayesian move ranking training with k-fold cross validation
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
function run(lib, file_name, no_folds)
    load_chess_games = dlsym(lib, :load_chess_games)
    initialize_attack_boards = dlsym(lib, :initialize_attack_boards)
    initialize_helper_boards = dlsym(lib, :initialize_helper_boards)
    initialize_zobrist_table = dlsym(lib, :initialize_zobrist_table)
    initialize_ranking_updates = dlsym(lib, :initialize_ranking_updates)
    initialize_move_zobrist_table = dlsym(lib, :initialize_move_zobrist_table)
    initialize_ht_urgencies = dlsym(lib, :initialize_ht_urgencies)
    train_model = dlsym(lib, :train_model)
    get_no_keys = dlsym(lib, :get_no_keys)
    deletes_ht_urgencies = dlsym(lib, :deletes_ht_urgencies)
    delete_chess_games = dlsym(lib, :delete_chess_games)

    # trains an urgency table with the C-code engine
    function train(games::ChessGames)
        # allocate a new hash table for urgencies
        urgencies = @ccall $initialize_ht_urgencies()::Ptr{HTUrgencies}

        # call the training function
        train_info = TrainingInfo(urgencies, CGaussian(0.0,1.0), 0.5, 0, C_NULL, 1)
        @ccall $train_model(
            games.games::Ptr{Ptr{ChessGame}},
            games.no_games::Cint,
            train_info::TrainingInfo,
        )::Cvoid
        no_unique_moves = @ccall $get_no_keys(urgencies::Ptr{HTUrgencies})::Cint
        println("Unique moves: $no_unique_moves")

        # release the hash table for urgencies
        @ccall $deletes_ht_urgencies(urgencies::Ptr{HTUrgencies})::Cvoid
    end

    # initalizes the chess engine
    @ccall $initialize_attack_boards()::Cvoid
    @ccall $initialize_helper_boards()::Cvoid
    @ccall $initialize_zobrist_table()::Cvoid
    @ccall $initialize_ranking_updates()::Cvoid
    @ccall $initialize_move_zobrist_table()::Cvoid

    # load the chess games from the file
    chess_games = @ccall $load_chess_games(file_name::Cstring)::ChessGames

    # generate the folds
    fold_size = chess_games.no_games ÷ no_folds
    start_idx = 1

    for i = 1:no_folds
        # determine end index of the fold
        if i == no_folds
            end_idx = chess_games.no_games
        else
            end_idx = start_idx + (fold_size - 1)
        end

        # iterate over all chess games to generate the folds
        training_set = Vector{Ptr{ChessGame}}(undef, chess_games.no_games - (end_idx - start_idx + 1))
        test_set = Vector{Ptr{ChessGame}}(undef, end_idx - start_idx + 1)
        train_idx = 1
        test_idx = 1
        for i = 1:chess_games.no_games
            if i >= start_idx && i <= end_idx
                test_set[test_idx] = unsafe_load(chess_games.games, i)
                test_idx += 1
            else
                training_set[train_idx] = unsafe_load(chess_games.games, i)
                train_idx += 1
            end
        end

        fold_training_games = ChessGames(chess_games.no_games - (end_idx - start_idx + 1), pointer(training_set))
        train(fold_training_games)

        start_idx = end_idx + 1
    end

    # release the games
    @ccall $delete_chess_games(chess_games::ChessGames)::Cvoid
end

lib = dlopen("lib/libchess.so")
try
    # run(lib, pwd() * "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn", 10)
    run(lib, "/tmp/short_games.pgn", 5)
finally
    dlclose(lib)
end

# Tool to test the accuracy of Bayesian move ranking training with k-fold cross validation
#
# 2023 written by Ralf Herbrich

using Plots
using Statistics
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

# overload the < operator for sorting
function Base.:<(a::CGaussian, b::CGaussian)
    return a.τ / a.ρ < b.τ / b.ρ
end

# placeholder for a hash table of urgency beliefs
struct HTUrgencies end

# corresponding structure for a single move set
struct MoveSet
    move_idx::Cint
    no_moves::Cint
    moves::Ptr{Cint}
end

# corresponding structure for an array of move sets
struct MoveSetArray
    no_move_sets::Cint
    capacity::Cint
    move_sets::Ptr{Ptr{MoveSet}}
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
function run(lib, file_name, no_folds; prior = CGaussian(0.0, 1.0))
    load_chess_games = dlsym(lib, :load_chess_games)
    initialize_attack_boards = dlsym(lib, :initialize_attack_boards)
    initialize_helper_boards = dlsym(lib, :initialize_helper_boards)
    initialize_zobrist_table = dlsym(lib, :initialize_zobrist_table)
    initialize_ranking_updates = dlsym(lib, :initialize_ranking_updates)
    initialize_move_zobrist_table = dlsym(lib, :initialize_move_zobrist_table)
    initialize_ht_urgencies = dlsym(lib, :initialize_ht_urgencies)
    games_to_move_sets = dlsym(lib, :games_to_move_sets)
    train_model = dlsym(lib, :train_model)
    get_no_keys = dlsym(lib, :get_no_keys)
    deletes_ht_urgencies = dlsym(lib, :deletes_ht_urgencies)
    delete_chess_games = dlsym(lib, :delete_chess_games)
    delete_move_set_array = dlsym(lib, :delete_move_set_array)
    get_urgency = dlsym(lib, :get_urgency)

    # trains an urgency table with the C-code engine
    function train(games::ChessGames, urgencies::Ptr{HTUrgencies})
        # call the training function
        train_info = TrainingInfo(urgencies, prior, 0.5, 0, C_NULL, 1)
        @ccall $train_model(
            games.games::Ptr{Ptr{ChessGame}},
            games.no_games::Cint,
            train_info::TrainingInfo,
        )::Cvoid
        no_unique_moves = @ccall $get_no_keys(urgencies::Ptr{HTUrgencies})::Cint
        println("Unique moves: $no_unique_moves")
    end

    # tests the trained urgency table with the C-code engine
    function test(games::ChessGames, urgencies::Ptr{HTUrgencies})
        # convert each move set into a set of Gaussians
        msa = @ccall $games_to_move_sets(games::ChessGames)::Ptr{MoveSetArray}
        test_move_sets = unsafe_load(msa, 1)

        all_move_urgencies =
            Vector{Tuple{Int,Vector{CGaussian}}}(undef, test_move_sets.no_move_sets)
        for i = 1:test_move_sets.no_move_sets
            move_urgencies = Vector{CGaussian}()

            ms = unsafe_load(unsafe_load(test_move_sets.move_sets, i), 1)
            for j = 1:ms.no_moves
                move_key = unsafe_load(ms.moves, j)
                ptr = @ccall $get_urgency(
                    urgencies::Ptr{HTUrgencies},
                    move_key::Cint,
                )::Ptr{CGaussian}
                push!(move_urgencies, (ptr == C_NULL) ? prior : unsafe_load(ptr, 1))
            end

            all_move_urgencies[i] = (ms.move_idx, move_urgencies)
        end
        @ccall $delete_move_set_array(msa::Ptr{MoveSetArray})::Cvoid

        # generate result file
        move_predictions = Vector{Tuple{Int,Int,CGaussian}}()
        for i in eachindex(all_move_urgencies)
            move_idx, move_urgencies = all_move_urgencies[i]
            t = (
                move_idx,
                findall(x -> x == 1, sortperm(move_urgencies, lt = Base.:<, rev = true))[1],
                move_urgencies[1]
            )
            push!(move_predictions, t)
        end

        return move_predictions
    end

    # initialize move move_predictions
    fold_move_predicitions = Vector{Vector{Tuple{Int,Int,CGaussian}}}()

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
        training_set =
            Vector{Ptr{ChessGame}}(undef, chess_games.no_games - (end_idx - start_idx + 1))
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
        println(
            "Fold $i: $(length(training_set)) training games, $(length(test_set)) test games",
        )

        fold_training_games = ChessGames(
            chess_games.no_games - (end_idx - start_idx + 1),
            pointer(training_set),
        )
        fold_test_games = ChessGames(end_idx - start_idx + 1, pointer(test_set))

        # allocate a new hash table for urgencies
        urgencies = @ccall $initialize_ht_urgencies()::Ptr{HTUrgencies}

        # train the urgencies and tests them right 
        train(fold_training_games, urgencies)
        mp = test(fold_test_games, urgencies)
        push!(fold_move_predicitions, mp)

        # release the hash table for urgencies
        @ccall $deletes_ht_urgencies(urgencies::Ptr{HTUrgencies})::Cvoid

        start_idx = end_idx + 1
    end

    # release the games
    @ccall $delete_chess_games(chess_games::ChessGames)::Cvoid

    # return the move move_predictions
    return fold_move_predicitions
end

lib = dlopen("lib/libchess.so")
try
    move_preds = run(lib, pwd() * "/data/ficsgamesdb_2022_standard2000_nomovetimes_288254.pgn", 10)
    # move_preds = run(lib, "/tmp/many_games.pgn", 10)
    mp = vcat(move_preds...)

    # plot scatter plot of the rank accuracy
    p = plot(
        xlabel = "Move index",
        ylabel = "Rank Accuracy",
        legend = :topright,
        ylim = (0, 1),
        xlim = (1, 150),
    )
    for rank_pos in [1;3;5;10;20]
        # compute the plot numbers
        max_no_moves = maximum(x -> x[1], mp)
        rank_accuracy = Vector{Float64}(undef, max_no_moves)
        for move_idx = 1:max_no_moves
            idx = findall(x -> x[1] == move_idx, mp)
            # rank_accuracy[move_idx] = mean(x -> x[2], mp[idx])
            if length(idx) > 0
                rank_accuracy[move_idx] = length(findall(x -> x[2] <= rank_pos, mp[idx])) / length(idx)
            else
                rank_accuracy[rank_pos] = 0
            end
        end

        # plot scatter plot of the rank accuracy
        scatter!(
            rank_accuracy,
            label = "Top-$rank_pos",
        )
    end
    display(p)
finally
    dlclose(lib)
end

# # compute the plot numbers
# max_no_rank = maximum(x -> x[2], mp)
# mean_belief = Vector{Float64}(undef, max_no_rank)
# for rank = 1:max_no_rank
#     idx = findall(x -> x[2] == rank, mp)
#     mean_belief[rank] = mean(x -> x[3].τ/x[3].ρ, mp[idx])
# end

# p = scatter(
#     1:max_no_rank,
#     mean_belief,
#     xlabel = "Rank",
#     ylabel = "Mean Urgency",
#     legend = false,
# )
# display(p)

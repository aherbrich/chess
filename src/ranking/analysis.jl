using Plots
using Statistics

# Analyses the results of the ranking algorithm
function create_dict_rank_count()
    # create dictionary which holds the rank of expert moves
    rank_count = Dict{Int64,Int64}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank of expert move
        _, rank = match(r"([0-9]+) ([0-9]+)", m)
        rank = parse(Int64, rank)
        # add rank to dictionary
        if haskey(rank_count, rank)
            rank_count[rank] += 1
        else
            rank_count[rank] = 1
        end
    end

    #sort dictionary by number of occurrences for a rank
    rank_count = sort(collect(rank_count), by=x->x[2], rev=true)
    return rank_count
end

# plot the results  
function plot_rank_count()
    d = create_dict_rank_count()
    d= d[1:40]

    # extract ranks and number of occurrences
    r = [x[1] for x in d]
    count = [x[2] for x in d]

    # extract relative number of occurrences
    rel = count ./ sum(count)

    # calculate cumulative sum of relative number of occurrences
    cum = cumsum(rel)

    # plot the results
    p = bar(
        r,
        cum,
        label = "cumulative",
        xlabel = "rank",
        ylabel = "Cumulative relative frequency",
        title = "Expert move is in top n ranks",
        legend = :topleft,
        yaxis = [0,1]
    )
    display(p)
end


function create_dict_move_to_rank()
    # create dictionary which holds move number and average rank
    rank_grouped = Dict{Int64, Vector{Int64}}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank and move number 
        move_nr, rank = match(r"([0-9]+) ([0-9]+)", m)
        rank = parse(Int64, rank)
        move_nr = parse(Int64, move_nr)

        # add move number and rank to dictionary
        if haskey(rank_grouped, move_nr)
            push!(rank_grouped[move_nr], rank)
        else
            rank_grouped[move_nr] = [rank]
        end
    end

    # calculate average rank and variance of rank for each move number

    rank_count = Dict{Int64, Tuple{Float64, Float64}}()
    for (k,v) in rank_grouped
        rank_count[k] = (mean(v), sqrt(var(v)))
    end

    rank_count = sort(collect(rank_count), by=x->x[1])

    return rank_count
end

function create_dict_move_to_possible()
    # create dictionary which holds move number and average rank
    rank_grouped = Dict{Int64, Vector{Int64}}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank and move number 
        move_nr, _, possible = match(r"([0-9]+) ([0-9]+) ([0-9]+)", m)
        move_nr = parse(Int64, move_nr)
        possible = parse(Int64, possible)

        # add move number and rank to dictionary
        if haskey(rank_grouped, move_nr)
            push!(rank_grouped[move_nr], possible)
        else
            rank_grouped[move_nr] = [possible]
        end
    end

    # calculate average rank and variance of rank for each move number

    rank_count = Dict{Int64, Float64}()
    for (k,v) in rank_grouped
        rank_count[k] = mean(v)
    end

    rank_count = sort(collect(rank_count), by=x->x[1])

    return rank_count
end

function create_dict_move_to_heuristic()
    # create dictionary which holds move number and average rank
    rank_grouped = Dict{Int64, Vector{Int64}}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank and move number 
        move_nr, _, _, _, rank_heuristic = match(r"([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)", m)
        move_nr = parse(Int64, move_nr)
        rank_heuristic = parse(Int64, rank_heuristic)

        # add move number and rank to dictionary
        if haskey(rank_grouped, move_nr)
            push!(rank_grouped[move_nr], rank_heuristic)
        else
            rank_grouped[move_nr] = [rank_heuristic]
        end
    end

    # calculate average rank and variance of rank for each move number

    rank_count = Dict{Int64, Float64}()
    for (k,v) in rank_grouped
        rank_count[k] = mean(v)
    end

    rank_count = sort(collect(rank_count), by=x->x[1])

    return rank_count
end

function create_dict_move_to_random_choice()
    # create dictionary which holds move number and average rank
    rank_grouped = Dict{Int64, Vector{Int64}}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank and move number 
        move_nr, _, _, random_choice = match(r"([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)", m)
        move_nr = parse(Int64, move_nr)
        random_choice = parse(Int64, random_choice)

        # add move number and rank to dictionary
        if haskey(rank_grouped, move_nr)
            push!(rank_grouped[move_nr], random_choice)
        else
            rank_grouped[move_nr] = [random_choice]
        end
    end

    # calculate average rank and variance of rank for each move number

    rank_count = Dict{Int64, Float64}()
    for (k,v) in rank_grouped
        rank_count[k] = mean(v)
    end

    rank_count = sort(collect(rank_count), by=x->x[1])

    return rank_count
end


function plot_move_to_rank()
    d = create_dict_move_to_rank()
    d2 = create_dict_move_to_possible()
    d3 = create_dict_move_to_random_choice()
    d4 = create_dict_move_to_heuristic()

    # extract average rank and move number
    move_nrs = [x[1] for (idx, x) in enumerate(d)]
    bayesian = [x[2][1]  for (idx, x) in enumerate(d)]
    bayesian_std = [x[2][2] for (idx, x) in enumerate(d)]
    possible = [x[2] for (idx, x) in enumerate(d2)]
    random_choice = [x[2] for (idx, x) in enumerate(d3)]
    heuristic = [x[2] for (idx, x) in enumerate(d4)]

    # plot the results
    p = scatter(move_nrs, bayesian, alpha=0.8, markersize=2, label ="Average rank", xlabel="Move number", ylabel="Rank/Possible moves", title="Rank of expert move/possible moves over time", color="red")
    scatter!(move_nrs, possible, alpha=0.8, markersize=2, label ="Average moves possible", color="orange")
    scatter!(move_nrs, random_choice, alpha=0.8, markersize=2, label ="Average rank by random choice", color="blue")
    scatter!(move_nrs, heuristic, alpha=0.8, markersize=2, label ="Average rank by heuristic choice", color="green")
    # savefig(p,"figure.png")
end
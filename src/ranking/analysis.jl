using Plots
using Statistics

# Analyses the results of the ranking algorithm
function create_dict_rank_count()
    # create dictionary which holds the rank of expert moves
    rank_count = Dict{Int64,Int64}()
    # read test output files
    for m in eachline("tmp/output_test_1.txt")
        # extract rank of expert move
        for c in eachmatch(r"^\d+", m)
            rank = parse(Int64, c.match)
            # add rank to dictionary
            if haskey(rank_count, rank)
                rank_count[rank] += 1
            else
                rank_count[rank] = 1
            end
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
        rank, move_nr = match(r"([0-9]+) ([0-9]+)", m)
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


function plot_move_to_rank()
    d = create_dict_move_to_rank()
    
    # extract average rank and move number
    move_nrs = [x[1] for x in d]
    means = [x[2][1]  for x in d]
    std = [x[2][2] for x in d]

    moves = []
    ranks = []
    for m in eachline("tmp/output_test_1.txt")
        # extract rank and move number 
        rank, nr = match(r"([0-9]+) ([0-9]+)", m)
        rank = parse(Int64, rank)
        nr = parse(Int64, nr)

        # add rank and move_nr to vector
        push!(moves, nr)
        push!(ranks, rank)
    end
    
    #scatter(moves, ranks, alpha=0.02, markersize=2, label= "MoveNr/Rank pair", xlabel="Move number", ylabel="Rank", title="Rank of expert move over time", color="green")
    scatter(move_nrs, means, yerr=std, alpha=0.8, markersize=2, label ="Average rank", xlabel="Move number", ylabel="Rank", title="Rank of expert move over time", color="red")
end
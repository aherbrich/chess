# Trains a Bayesian Ranking model on the given data set
#
# 2023 by Ralf Herbrich
# Hasso-Plattner Institute

using Plots

include("ranking.jl")

# reads /tmp/moves.dat file line by line
function read_moves(filename)
    d_all_moves = Dict{String,Int}()
    d_moves_made = Dict{String,Int}()

    open(filename) do f
        reg = r"\(([pnbrqkPNBRQKE]-[a-h][1-8]\ [pnbrqkPNBRQKE]-[a-h][1-8][01][nbrqNBRQE])\)"
        for line in eachline(f)
            moves_cnt = 0
            for m in eachmatch(reg, line)
                if (moves_cnt == 0)
                    # update the dictionary of all moves
                    if (haskey(d_moves_made, m.captures[1]))
                        d_moves_made[m.captures[1]] += 1
                    else
                        d_moves_made[m.captures[1]] = 1
                    end
                end

                moves_cnt += 1

                # update the dictionary of all moves
                if (haskey(d_all_moves, m.captures[1]))
                    d_all_moves[m.captures[1]] += 1
                else
                    d_all_moves[m.captures[1]] = 1
                end
            end
        end
    end

    return (d_all_moves, d_moves_made)
end

# reads /tmp/moves.dat file line by line
function read_moves_with_missing_mass(filename)
    d = Dict{String,Int}()
    n = 0
    missing_mass = Vector{Float64}()
    open(filename) do f
        reg = r"\(([pnbrqkPNBRQKE]-[a-h][1-8]\ [pnbrqkPNBRQKE]-[a-h][1-8][01][nbrqNBRQE])\)"
        block_line_counter = 0
        moves_read_in_block = 0
        moves_not_found_in_block = 0

        for line in eachline(f)
            for m in eachmatch(reg, line)
                n += 1
                moves_read_in_block += 1
                if (haskey(d, m.captures[1]))
                    d[m.captures[1]] += 1
                else
                    moves_not_found_in_block += 1
                    d[m.captures[1]] = 1
                end
            end

            block_line_counter += 1
            if (mod(block_line_counter, 10000) == 0)
                push!(missing_mass, moves_not_found_in_block / moves_read_in_block * 100.0)
                println(
                    "Read $n moves, found $moves_read_in_block moves, $moves_not_found_in_block moves not found: $(moves_not_found_in_block/moves_read_in_block*100)",
                )
                moves_read_in_block = 0
                moves_not_found_in_block = 0
            end
        end
    end

    p = plot(
        missing_mass,
        label = "missing mass",
        xlabel = "block",
        ylabel = "missing mass",
        title = "Missing mass per block",
        legend = :topleft,
    )
    display(p)

    return (n, d)
end

# loop over dictionary and count the number of occurences
function count_moves(d)
    n = 0
    for (k, v) in d
        n += v
    end
    return (n)
end

# sort the dictionary by the number of occurences
function sort_moves(d)
    return (sort(collect(d), by = x -> x[2], rev = true))
end

# plot a histogram of the moves based on the number of occurences
function plot_moves(d, n)
    moves = sort_moves(d)
    moves = moves[1:n]
    moves = [(x[1], x[2]) for x in moves]
    bar(
        moves,
        label = "moves",
        xlabel = "move",
        ylabel = "occurences",
        title = "Top 20 moves",
        legend = false,
        yaxis = :log,
    )
end


# sort the dictionary by the number of occurences
function sort_moves_made(d_moves_made, d_all_moves)
    # extend the moves made by the count of occurance in d_all_moves
    d_moves_made_with_overall_count = Dict{String,Tuple{Int,Int}}()
    for (k, v) in d_moves_made
        d_moves_made_with_overall_count[k] = (v, d_all_moves[k])
    end
    return (sort(
        collect(d_moves_made_with_overall_count),
        by = x -> x[2][1] / x[2][2],
        rev = true,
    ))
end

# plots the coverage of the top-n moves
function plot_coverage(d, n)
    total = count_moves(d)
    d2 = sort_moves(d)
    p = Vector{Float64}()
    s = 0
    i = 0
    for (k, v) in d2
        s += v
        push!(p, s / total)
        i += 1
        if (i >= n)
            break
        end
    end

    plot(
        p,
        linewidth = 3,
        xlabel = "move",
        ylabel = "coverage",
        title = "Coverage of top $n moves",
        legend = false,
        ylim = (0, 1),
    )
end

# trains a model from the given data set
function train_model(filename; n = 1000, β = 25.0 / 6.0, μ = 25.0, σ = 25.0 / 3.0)
    d_all_moves = Dict{String,Gaussian1D}()
    lines_read = 0

    open(filename) do f
        reg =
            r"\(([pnbrqkPNBRQKE]-[a-h][1-8]\ [pnbrqkPNBRQKE]-[a-h][1-8][nbrqNBRQE]\ [01])\)"
        for line in eachline(f)
            prior_beliefs = Vector{Gaussian1D}()
            for m in eachmatch(reg, line)
                # extract the current beliefs
                if (haskey(d_all_moves, m.captures[1]))
                    push!(prior_beliefs, d_all_moves[m.captures[1]])
                else
                    push!(prior_beliefs, Gaussian1Dμσ2(μ, σ * σ))
                end
            end

            # run the ranking update
            (P, posterior_beliefs) = ranking_update(prior_beliefs, β)

            # write back the posterior beliefs
            i = 1
            for m in eachmatch(reg, line)
                d_all_moves[m.captures[1]] = posterior_beliefs[i]
                i += 1
            end

            # update lines_read
            lines_read += 1
            if (lines_read == n)
                break
            end
        end
    end

    return d_all_moves
end

# output the moves with the highest belief
function sort_moves_by_belief(d, moves, β)
    tmp = Vector{Tuple{String,Gaussian1D}}()
    for m in moves
        push!(tmp, (m, d[m]))
    end
    post_beliefs = move_probability(map(x -> x[2], tmp), β)

    move_with_probs = Vector{Tuple{String,Float64}}(undef, length(tmp))
    for i in eachindex(tmp)
        move_with_probs[i] = (tmp[i][1], post_beliefs[i])
    end

    return (sort(move_with_probs, by = x -> x[2], rev = true))
end

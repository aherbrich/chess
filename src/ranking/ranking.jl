# Library for move ranking algorithm
#
# 2023 by Ralf Herbrich
# Hasso-Plattner Institute

include("factors.jl")

# Computes the TrueSkills for a two-player game
function ranking_update(move_made::Gaussian1D, other_moves::Vector{Gaussian1D}, β)
    bag = DistributionBag(Gaussian1D(0, 0))
    factorList = Vector{Factor}()

    # helper function to add factors to a long list
    function addFactor(f)
        push!(factorList, f)
        return (f)
    end

    # Urgency and latent urgency variables for the winning move
    u1 = add!(bag)
    x1 = add!(bag)

    # Urgency and latent urgency variables as well as pairwise difference for the other move
    us = Vector{Int}(undef, length(other_moves))
    xs = Vector{Int}(undef, length(other_moves))
    ds = Vector{Int}(undef, length(other_moves))
    for i in eachindex(other_moves)
        us[i] = add!(bag)
        xs[i] = add!(bag)
        ds[i] = add!(bag)
    end

    # Gaussian prior for the urgency of the winning move and the other moves
    priorU1 = addFactor(GaussianFactor(move_made, u1, bag))
    priorUs = Vector{Factor}(undef, length(other_moves))
    for i in eachindex(other_moves)
        priorUs[i] = addFactor(GaussianFactor(other_moves[i], us[i], bag))
    end

    # Gaussian noise for the latent urgencies of the winning move and the other moves
    likelU1 = addFactor(GaussianMeanFactor(β * β, x1, u1, bag))
    likelUs = Vector{Factor}(undef, length(other_moves))
    diffUs = Vector{Factor}(undef, length(other_moves))
    compUs = Vector{Factor}(undef, length(other_moves))
    for i in eachindex(other_moves)
        likelUs[i] = addFactor(GaussianMeanFactor(β * β, xs[i], us[i], bag))
        diffUs[i] = addFactor(WeightedSumFactor(1, -1, x1, xs[i], ds[i], bag))
        compUs[i] = addFactor(GreaterThanFactor(0, ds[i], bag))
    end

    # run the message passing schedule
    priorU1.update!(1)
    likelU1.update!(1)
    for i in eachindex(other_moves)
        priorUs[i].update!(1)
        likelUs[i].update!(1)
    end

    Δ = 1e4
    while (Δ > 1e-6)
        Δ = 0
        for i in eachindex(other_moves)
            Δ = max(Δ, diffUs[i].update!(3))
            Δ = max(Δ, compUs[i].update!(1))
            Δ = max(Δ, diffUs[i].update!(1))
            Δ = max(Δ, diffUs[i].update!(2))
        end
    end

    # finally, send the messages back to the prior 
    likelU1.update!(2)
    for i in eachindex(other_moves)
        likelUs[i].update!(2)
    end

    # and now compute the log normalization constant
    println("Z = ", exp(logNormalization(factorList, bag)))

    return (bag[u1], map(i -> bag[i], us))
end


β = 25.0 / 6.0

println("\n\nTwo Move example\n=================")
move_made = Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))
other_moves = [Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
(post_move_made, post_other_moves) = ranking_update(move_made, other_moves, β)
println(move_made, " (Winner) ==> ", post_move_made)
println(other_moves[1], " (Losers) ==> ", post_other_moves[1])

println("\n\nThree Move example\n=================")
move_made = Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))
other_moves = [Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)), Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
(post_move_made, post_other_moves) = ranking_update(move_made, other_moves, β)
println(move_made, " (Winner) ==> ", post_move_made)
println(other_moves[1], " (Loser 1) ==> ", post_other_moves[1])
println(other_moves[2], " (Loser 2) ==> ", post_other_moves[2])


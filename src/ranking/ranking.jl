# Library for move ranking algorithm
#
# 2023 by Ralf Herbrich
# Hasso-Plattner Institute

include("factors.jl")

# Computes the TrueSkills for a two-player game
function ranking_update(move_beliefs::Vector{Gaussian1D}, β)
    bag = DistributionBag(Gaussian1D(0, 0))
    factorList = Vector{Factor}()

    # helper function to add factors to a long list
    function addFactor(f)
        push!(factorList, f)
        return (f)
    end

    # Urgency and latent urgency variables as well as pairwise difference for all moves
    urgency = Vector{Int}(undef, length(move_beliefs))
    latent_urgency = Vector{Int}(undef, length(move_beliefs))
    diff = Vector{Int}(undef, length(move_beliefs)-1)
    for i in eachindex(move_beliefs)
        urgency[i] = add!(bag)
        latent_urgency[i] = add!(bag)
        if (i < length(move_beliefs))
            diff[i] = add!(bag)
        end
    end

    # Gaussian prior for the urgency of all moves
    priorF = Vector{Factor}(undef, length(move_beliefs))
    for i in eachindex(move_beliefs)
        priorF[i] = addFactor(GaussianFactor(move_beliefs[i], urgency[i], bag))
    end

    # Gaussian noise for the latent urgencies of the winning move and the other moves
    likelF = Vector{Factor}(undef, length(move_beliefs))
    diffF = Vector{Factor}(undef, length(move_beliefs)-1)
    compareF = Vector{Factor}(undef, length(move_beliefs)-1)
    for i in eachindex(move_beliefs)
        likelF[i] = addFactor(GaussianMeanFactor(β * β, latent_urgency[i], urgency[i], bag))
        if (i < length(move_beliefs))
            diffF[i] = addFactor(WeightedSumFactor(1, -1, latent_urgency[1], latent_urgency[i+1], diff[i], bag))
            compareF[i] = addFactor(GreaterThanFactor(0, diff[i], bag))
        end
    end

    # run the message passing schedule
    for i in eachindex(move_beliefs)
        priorF[i].update!(1)
        likelF[i].update!(1)
    end

    Δ = 1e4
    while (Δ > 1e-6)
        Δ = 0
        for i in eachindex(move_beliefs)
            if (i < length(move_beliefs))
                Δ = max(Δ, diffF[i].update!(3))
                Δ = max(Δ, compareF[i].update!(1))
                Δ = max(Δ, diffF[i].update!(1))
                Δ = max(Δ, diffF[i].update!(2))
            end
        end
    end

    # finally, send the messages back to the prior 
    for i in eachindex(move_beliefs)
        likelF[i].update!(2)
    end

    # and now compute the log normalization constant
    # println("Z = ", exp(logNormalization(factorList, bag)))

    return (exp(logNormalization(factorList, bag)), map(i -> bag[i], urgency))
end

function test()
    β = 25.0 / 6.0

    println("\n\nTwo Move example\n=================")
    prior_beliefs = [
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)), 
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
    posterior_beliefs = ranking_update(prior_beliefs, β)
    println(prior_beliefs[1], " (Winner) ==> ", posterior_beliefs[1])
    println(prior_beliefs[2], " (Loser) ==> ", posterior_beliefs[2])

    println("\n\nThree Move example\n=================")
    prior_beliefs = [
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
    posterior_beliefs = ranking_update(prior_beliefs, β)
    println(prior_beliefs[1], " (Winner) ==> ", posterior_beliefs[1])
    println(prior_beliefs[2], " (Loser 1) ==> ", posterior_beliefs[2])
    println(prior_beliefs[3], " (Loser 2) ==> ", posterior_beliefs[3])
end

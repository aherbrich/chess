# Library for move ranking algorithm
#
# 2023 by Ralf Herbrich
# Hasso-Plattner Institute

include("factors.jl")

# Computes the ranking update for a single move made and returns the log-normalization constant and the posterior belief
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

    # return the log-normalization constant and the posterior beliefs
    return (logNormalization(factorList, bag), map(i -> bag[i], urgency))
end

# Computes the move probabilities
function move_probability(move_beliefs::Vector{Gaussian1D}, β)
    Z = Vector{Float64}(undef, length(move_beliefs))
    idx = collect(1:length(move_beliefs))
    for i in eachindex(move_beliefs)
        # swap the 1st and ith element of idx
        idx[1], idx[i] = idx[i], idx[1]

        (logP, _) = ranking_update(move_beliefs[idx], β)
        Z[i] = exp(logP)

        # swap back
        idx[1], idx[i] = idx[i], idx[1]
    end

    return Z
end

# Computes an approximation to the move probabilities
function move_probability_approx(move_beliefs::Vector{Gaussian1D}, β)
    Z = Vector{Float64}(undef, length(move_beliefs))
    for i in eachindex(move_beliefs)
        Z[i] = 1.0
        for j in eachindex(move_beliefs)
            if (i != j)
                Z[i] *= 1 - cdf(Normal(mean(move_beliefs[i]) - mean(move_beliefs[j]), 
                                       sqrt(variance(move_beliefs[i]) + variance(move_beliefs[j]) + 2 * β * β)), 0)
            end
        end
    end

    # renormalize Z
    Z = Z / sum(Z)

    return Z
end

# A series of tests
function test()
    β = 25.0 / 6.0

    println("\n\nTwo Move example\n=================")
    prior_beliefs = [
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)), 
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
    (Z, posterior_beliefs) = ranking_update(prior_beliefs, β)
    println(prior_beliefs[1], " (Winner) ==> ", posterior_beliefs[1])
    println(prior_beliefs[2], " (Loser) ==> ", posterior_beliefs[2])
    println("Z = ", exp(Z))

    println("\n\nThree Move example\n=================")
    prior_beliefs = [
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
        Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]
    (Z, posterior_beliefs) = ranking_update(prior_beliefs, β)
    println(prior_beliefs[1], " (Winner) ==> ", posterior_beliefs[1])
    println(prior_beliefs[2], " (Loser 1) ==> ", posterior_beliefs[2])
    println(prior_beliefs[3], " (Loser 2) ==> ", posterior_beliefs[3])
    println("Z = ", exp(Z))

    println("\n\nNormalization\n=================")
    prior_beliefs = [
        Gaussian1Dμσ2(41.57517524002317, 0.6605468334386351 * 0.6605468334386351),
        Gaussian1Dμσ2(37.332437754079756, 0.4705768648864796 * 0.4705768648864796),
        Gaussian1Dμσ2(36.888465029048206, 0.2991688936980313 * 0.2991688936980313)]
    # prior_beliefs = [
    #     Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
    #     Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0)),
    #     Gaussian1Dμσ2(25.0, 25.0 * 25.0 / (3.0 * 3.0))]

    P_true = move_probability(prior_beliefs, β)
    P_approx = move_probability_approx(prior_beliefs, β)

    println("P_true = ", P_true)
    println("P_approx = ", P_approx)
end

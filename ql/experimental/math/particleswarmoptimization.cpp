/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2015 Andres Hernandez

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<https://www.quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/math/particleswarmoptimization.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <cmath>
#include <utility>

using std::sqrt;

namespace QuantLib {
    ParticleSwarmOptimization::ParticleSwarmOptimization(Size M,
                                                         ext::shared_ptr<Topology> topology,
                                                         ext::shared_ptr<Inertia> inertia,
                                                         Real c1,
                                                         Real c2,
                                                         unsigned long seed)
    : M_(M), rng_(seed), topology_(std::move(topology)), inertia_(std::move(inertia)) {
        Real phi = c1 + c2;
        QL_ENSURE(phi*phi - 4 * phi != 0.0, "Invalid phi");
        c0_ = 2.0 / std::abs(2.0 - phi - sqrt(phi*phi - 4 * phi));
        c1_ = c0_*c1;
        c2_ = c0_*c2;
    }

    ParticleSwarmOptimization::ParticleSwarmOptimization(Size M,
                                                         ext::shared_ptr<Topology> topology,
                                                         ext::shared_ptr<Inertia> inertia,
                                                         Real omega,
                                                         Real c1,
                                                         Real c2,
                                                         unsigned long seed)
    : M_(M), c0_(omega), c1_(c1), c2_(c2), rng_(seed), topology_(std::move(topology)),
      inertia_(std::move(inertia)) {}

    void ParticleSwarmOptimization::startState(Problem &P, const EndCriteria &endCriteria) {
        QL_REQUIRE(topology_, "Invalid topology");
        QL_REQUIRE(inertia_, "Invalid inertia");
        N_ = P.currentValue().size();
        topology_->setSize(M_);
        inertia_->setSize(M_, N_, c0_, endCriteria);
        X_.reserve(M_);
        V_.reserve(M_);
        pBX_.reserve(M_);
        pBF_ = Array(M_);
        gBX_.reserve(M_);
        gBF_ = Array(M_);
        uX_ = P.constraint().upperBound(P.currentValue());
        lX_ = P.constraint().lowerBound(P.currentValue());
        Array bounds = uX_ - lX_;

        //Random initialization is done by Sobol sequence
        SobolRsg sobol(N_ * 2);

        //Prepare containers
        for (Size i = 0; i < M_; i++) {
            const SobolRsg::sample_type::value_type &sample = sobol.nextSequence().value;
            X_.emplace_back(N_, 0.0);
            Array& x = X_.back();
            V_.emplace_back(N_, 0.0);
            Array& v = V_.back();
            gBX_.emplace_back(N_, 0.0);
            for (Size j = 0; j < N_; j++) {
                //Assign X=lb+(ub-lb)*random
                x[j] = lX_[j] + bounds[j] * sample[2 * j];
                //Assign V=(ub-lb)*2*random-(ub-lb) -> between (lb-ub) and (ub-lb)
                v[j] = bounds[j] * (2.0*sample[2 * j + 1] - 1.0);
            }
            //Evaluate X and assign as personal best
            pBX_.push_back(X_.back());
            pBF_[i] = P.value(X_.back());
        }

        //init topology & inertia
        topology_->init(this);
        inertia_->init(this);
    }

    EndCriteria::Type ParticleSwarmOptimization::minimize(Problem &P, const EndCriteria &endCriteria) {
        QL_REQUIRE(!P.constraint().empty(), "PSO is a constrained optimizer");

        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        Size iteration = 0;
        Size iterationStat = 0;
        Size maxIteration = endCriteria.maxIterations();
        Size maxIStationary = endCriteria.maxStationaryStateIterations();
        Real bestValue = QL_MAX_REAL;
        Size bestPosition = 0;

        startState(P, endCriteria);
        //Set best value & position
        for (Size i = 0; i < M_; i++) {
            if (pBF_[i] < bestValue) {
                bestValue = pBF_[i];
                bestPosition = i;
            }
        }

        //Run optimization
        do {
            iteration++;
            iterationStat++;
            //Check if stopping criteria is met
            if (iteration > maxIteration || iterationStat > maxIStationary)
                break;

            //According to the topology, determine best global position
            topology_->findSocialBest();

            //Call inertia to change internal state
            inertia_->setValues();

            //Loop over particles
            for (Size i = 0; i < M_; i++) {
                Array& x = X_[i];
                Array& pB = pBX_[i];
                const Array& gB = gBX_[i];
                Array& v = V_[i];

                //Loop over dimensions
                for (Size j = 0; j < N_; j++) {
                    //Update velocity
                    v[j] += c1_*rng_.nextReal()*(pB[j] - x[j]) + c2_*rng_.nextReal()*(gB[j] - x[j]);
                    //Update position
                    x[j] += v[j];
                    //Enforce bounds on positions
                    if (x[j] < lX_[j]) {
                        x[j] = lX_[j];
                        v[j] = 0.0;
                    }
                    else if (x[j] > uX_[j]) {
                        x[j] = uX_[j];
                        v[j] = 0.0;
                    }
                }
                //Evaluate x
                Real f = P.value(x);
                if (f < pBF_[i]) {
                    //Update personal best
                    pBF_[i] = f;
                    pB = x;
                    //Check stationary condition
                    if (f < bestValue) {
                        bestValue = f;
                        bestPosition = i;
                        iterationStat = 0;
                    }
                }
            }
        } while (true);
        if (iteration > maxIteration)
            ecType = EndCriteria::MaxIterations;
        else
            ecType = EndCriteria::StationaryPoint;

        //Set result to best point
        P.setCurrentValue(pBX_[bestPosition]);
        P.setFunctionValue(bestValue);
        return ecType;
    }

    void AdaptiveInertia::setValues() {
        Real currBest = (*pBF_)[0];
        for (Size i = 1; i < M_; i++) {
            if (currBest >(*pBF_)[i]) currBest = (*pBF_)[i];
        }
        if (started_) { //First iteration leaves inertia unchanged
            if (currBest < best_) {
                best_ = currBest;
                adaptiveCounter--;
            }
            else {
                adaptiveCounter++;
            }
            if (adaptiveCounter > sh_) {
                c0_ = std::max(minInertia_, std::min(maxInertia_, c0_*0.5));
            }
            else if (adaptiveCounter < sl_) {
                c0_ = std::max(minInertia_, std::min(maxInertia_, c0_*2.0));
            }
        }
        else {
            best_ = currBest;
            started_ = true;
        }
        for (Size i = 0; i < M_; i++) {
            (*V_)[i] *= c0_;
        }
    }

    void KNeighbors::findSocialBest() {
        for (Size i = 0; i < M_; i++) {
            Real bestF = (*pBF_)[i];
            Size bestX = 0;
            //Search K_ neightbors upwards
            Size upper = std::min(i + K_, M_);
            //Search K_ neighbors downwards
            Size lower = std::max(i, K_ + 1) - K_ - 1;
            for (Size j = lower; j < upper; j++) {
                if ((*pBF_)[j] < bestF) {
                    bestF = (*pBF_)[j];
                    bestX = j;
                }
            }
            if (i + K_ >= M_) { //loop around if i+K >= M_
                for (Size j = 0; j < i + K_ - M_; j++) {
                    if ((*pBF_)[j] < bestF) {
                        bestF = (*pBF_)[j];
                        bestX = j;
                    }
                }
            }
            else if (i < K_) {//loop around from above
                for (Size j = M_ - (K_ - i) - 1; j < M_; j++) {
                    if ((*pBF_)[j] < bestF) {
                        bestF = (*pBF_)[j];
                        bestX = j;
                    }
                }
            }
            (*gBX_)[i] = (*pBX_)[bestX];
            (*gBF_)[i] = bestF;
        }
    }

    ClubsTopology::ClubsTopology(Size defaultClubs,
                                 Size totalClubs,
                                 Size maxClubs,
                                 Size minClubs,
                                 Size resetIteration,
                                 unsigned long seed)
    : totalClubs_(totalClubs), maxClubs_(maxClubs), minClubs_(minClubs),
      defaultClubs_(defaultClubs), resetIteration_(resetIteration), bestByClub_(totalClubs, 0),
      worstByClub_(totalClubs, 0), generator_(seed), distribution_(1, totalClubs_) {
        QL_REQUIRE(totalClubs_ >= defaultClubs_,
            "Total number of clubs must be larger or equal than default clubs");
        QL_REQUIRE(defaultClubs_ >= minClubs_,
            "Number of default clubs must be larger or equal than minimum clubs");
        QL_REQUIRE(maxClubs_ >= defaultClubs_,
            "Number of maximum clubs must be larger or equal than default clubs");
        QL_REQUIRE(totalClubs_ >= maxClubs_,
            "Total number of clubs must be larger or equal than maximum clubs");
    }

    void ClubsTopology::setSize(Size M) {
        M_ = M;

        if (defaultClubs_ < totalClubs_) {
            clubs4particles_ = std::vector<std::vector<bool> >(M_, std::vector<bool>(totalClubs_, false));
            particles4clubs_ = std::vector<std::vector<bool> >(totalClubs_, std::vector<bool>(M_, false));
            //Assign particles to clubs randomly
            for (Size i = 0; i < M_; i++) {
                std::vector<bool> &clubSet = clubs4particles_[i];
                for (Size j = 0; j < defaultClubs_; j++) {
                    Size index = distribution_(generator_);
                    while (clubSet[index]) { index = distribution_(generator_); }
                    clubSet[index] = true;
                    particles4clubs_[index][i] = true;
                }
            }
        }
        else {
            //Since totalClubs_ == defaultClubs_, then just initialize to true
            clubs4particles_ = std::vector<std::vector<bool> >(M_, std::vector<bool>(totalClubs_, true));
            particles4clubs_ = std::vector<std::vector<bool> >(totalClubs_, std::vector<bool>(M_, true));
        }
    }

    void ClubsTopology::findSocialBest() {
        //Update iteration
        iteration_++;
        bool reset = false;
        if (iteration_ == resetIteration_) {
            iteration_ = 0;
            reset = true;
        }

        //Find best by current club
        for (Size i = 0; i < totalClubs_; i++) {
            Real bestByClub = QL_MAX_REAL;
            Real worstByClub = -QL_MAX_REAL;
            Size bestP = 0;
            Size worstP = 0;
            const std::vector<bool> &particlesSet = particles4clubs_[i];
            for (Size j = 0; j < M_; j++) {
                if (particlesSet[j]) {
                    if (bestByClub >(*pBF_)[j]) {
                        bestByClub = (*pBF_)[j];
                        bestP = j;
                    }
                    else if (worstByClub < (*pBF_)[j]) {
                        worstByClub = (*pBF_)[j];
                        worstP = j;
                    }
                }
            }
            bestByClub_[i] = bestP;
            worstByClub_[i] = worstP;
        }

        //Update clubs && global best
        for (Size i = 0; i < M_; i++) {
            std::vector<bool> &clubSet = clubs4particles_[i];
            bool best = true;
            bool worst = true;
            Size currentClubs = 0;
            for (Size j = 0; j < totalClubs_; j++) {
                if (clubSet[j]) {
                    //If still thought of the best, check if best in club j
                    if (best && i != bestByClub_[j]) best = false;
                    //If still thought of the worst, check if worst in club j
                    if (worst && i != worstByClub_[j]) worst = false;
                    //Update currentClubs
                    currentClubs++;
                }
            }
            //Update clubs
            if (best) {
                //Leave random club
                leaveRandomClub(i, currentClubs);
            }
            else if (worst) {
                //Join random club
                joinRandomClub(i, currentClubs);
            }
            else if (reset && currentClubs != defaultClubs_) {
                //If membership != defaultClubs_, then leave or join accordingly
                if (currentClubs < defaultClubs_) {
                    //Join random club
                    joinRandomClub(i, currentClubs);
                }
                else {
                    //Leave random club
                    leaveRandomClub(i, currentClubs);
                }
            }

            //Update global best
            Real bestNeighborF = QL_MAX_REAL;
            Size bestNeighborX = 0;
            for (Size j = 0; j < totalClubs_; j++) {
                if (clubSet[j] && bestNeighborF >(*pBF_)[bestByClub_[j]]) {
                    bestNeighborF = (*pBF_)[bestByClub_[j]];
                    bestNeighborX = j;
                }
            }
            (*gBX_)[i] = (*pBX_)[bestNeighborX];
            (*gBF_)[i] = bestNeighborF;
        }
    }

    void ClubsTopology::leaveRandomClub(Size particle, Size currentClubs) {
        Size randIndex = distribution_(generator_, param_type(1, currentClubs));
        Size index = 1;
        std::vector<bool> &clubSet = clubs4particles_[particle];
        for (Size j = 0; j < totalClubs_; j++) {
            if (clubSet[j]) {
                if (index == randIndex) {
                    clubSet[j] = false;
                    particles4clubs_[j][particle] = false;
                    break;
                }
                index++;
            }
        }
    }

    void ClubsTopology::joinRandomClub(Size particle, Size currentClubs) {
        Size randIndex = totalClubs_ == currentClubs ? 1 :
            distribution_(generator_, param_type(1, totalClubs_ - currentClubs));
        Size index = 1;
        std::vector<bool> &clubSet = clubs4particles_[particle];
        for (Size j = 0; j < totalClubs_; j++) {
            if (!clubSet[j]) {
                if (index == randIndex) {
                    clubSet[j] = true;
                    particles4clubs_[j][particle] = true;
                    break;
                }
                index++;
            }
        }
    }
}


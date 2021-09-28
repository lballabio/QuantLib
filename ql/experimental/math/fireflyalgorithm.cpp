/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2015 Andres Hernandez

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/math/fireflyalgorithm.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace QuantLib {
    FireflyAlgorithm::FireflyAlgorithm(Size M,
                                       ext::shared_ptr<Intensity> intensity,
                                       ext::shared_ptr<RandomWalk> randomWalk,
                                       Size Mde,
                                       Real mutation,
                                       Real crossover,
                                       unsigned long seed)
    : mutation_(mutation), crossover_(crossover), M_(M), Mde_(Mde), Mfa_(M_ - Mde_),
      intensity_(std::move(intensity)), randomWalk_(std::move(randomWalk)),
      drawIndex_(base_generator_type(seed), uniform_integer(Mfa_, Mde > 0 ? M_ - 1 : M_)),
      rng_(seed) {
        QL_REQUIRE(M_ >= Mde_,
            "Differential Evolution subpopulation cannot be larger than total population");
    }

    void FireflyAlgorithm::startState(Problem &P, const EndCriteria &endCriteria) {
        N_ = P.currentValue().size();
        x_.reserve(M_);
        xI_.reserve(M_);
        xRW_.reserve(M_);
        values_.reserve(M_);
        uX_ = P.constraint().upperBound(P.currentValue());
        lX_ = P.constraint().lowerBound(P.currentValue());
        Array bounds = uX_ - lX_;

        //Random initialization is done by Sobol sequence
        SobolRsg sobol(N_);

        //Prepare containers
        for (Size i = 0; i < M_; i++) {
            const SobolRsg::sample_type::value_type &sample = sobol.nextSequence().value;
            x_.emplace_back(N_, 0.0);
            xI_.emplace_back(N_, 0.0);
            xRW_.emplace_back(N_, 0.0);
            Array& x = x_.back();
            for (Size j = 0; j < N_; j++) {
                //Assign X=lb+(ub-lb)*random
                x[j] = lX_[j] + bounds[j] * sample[j];
            }
            //Evaluate point
            values_.emplace_back(P.value(x), i);
        }

        //init intensity & randomWalk
        intensity_->init(this);
        randomWalk_->init(this);
    }

    EndCriteria::Type FireflyAlgorithm::minimize(Problem &P, const EndCriteria &endCriteria) {
        QL_REQUIRE(!P.constraint().empty(), "Firefly Algorithm is a constrained optimizer");
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        Size iteration = 0;
        Size iterationStat = 0;
        Size maxIteration = endCriteria.maxIterations();
        Size maxIStationary = endCriteria.maxStationaryStateIterations();
        
        startState(P, endCriteria);

        bool isFA = Mfa_ > 0;
        //Variables for DE
        Array z(N_, 0.0);
        Size indexR1, indexR2;
        uniform_integer::param_type nParam(0, N_ - 1);

        //Set best value & position
        Real bestValue = values_[0].first;
        Size bestPosition = 0;
        for (Size i = 1; i < M_; i++) {
            if (values_[i].first < bestValue) {
                bestPosition = i;
                bestValue = values_[i].first;
            }
        }
        Array bestX = x_[bestPosition];

        //Run optimization
        do {
            iteration++;
            iterationStat++;
            //Check if stopping criteria is met
            if (iteration > maxIteration || iterationStat > maxIStationary)
                break;

            //Divide into two subpopulations
            //First sort values
            std::sort(values_.begin(), values_.end());

            //Differential evolution
            if(Mfa_ < M_){
                Size indexBest = values_[0].second;
                Array& xBest = x_[indexBest];
                for (Size i = Mfa_; i < M_; i++) { 
                    if (!isFA) {
                        //Pure DE requires random index
                        indexBest = drawIndex_();
                        xBest = x_[indexBest];
                    }
                    do { 
                        indexR1 = drawIndex_(); 
                    } while(indexR1 == indexBest);
                    do { 
                        indexR2 = drawIndex_(); 
                    } while(indexR2 == indexBest || indexR2 == indexR1);
                    
                    Size index = values_[i].second;
                    Array& x   = x_[index];
                    Array& xR1 = x_[indexR1];
                    Array& xR2 = x_[indexR2];
					Size rIndex = drawIndex_(nParam);
                    for (Size j = 0; j < N_; j++) {
                        if (j == rIndex || rng_.nextReal() <= crossover_) {
                            //Change x[j] according to crossover
                            z[j] = xBest[j] + mutation_*(xR1[j] - xR2[j]);
                        } else {
                            z[j] = x[j];
                        }
                        //Enforce bounds on positions
                        if (z[j] < lX_[j]) {
                            z[j] = lX_[j];
                        }
                        else if (z[j] > uX_[j]) {
                            z[j] = uX_[j];
                        }
                    }
                    Real val = P.value(z);
                    if (val < values_[index].first) {
                        //Accept new point
                        x = z;
                        values_[index].first = val;
                        //mark best
                        if (val < bestValue) {
                            bestValue = val;
                            bestX = x;
                            iterationStat = 0;
                        }
                    }
                }
            }
                
            //Firefly algorithm
            if(isFA){
                //According to the intensity, determine best global position
                intensity_->findBrightest();

                //Prepare random walk
                randomWalk_->walk();

                //Loop over particles
                for (Size i = 0; i < Mfa_; i++) {
                    Size index = values_[i].second;
                    Array& x   = x_[index];
                    Array& xI  = xI_[index];
                    Array& xRW = xRW_[index];

                    //Loop over dimensions
                    for (Size j = 0; j < N_; j++) {
                        //Update position
                        z[j] = x[j] + xI[j] + xRW[j];
                        //Enforce bounds on positions
                        if (z[j] < lX_[j]) {
                            z[j] = lX_[j];
                        }
                        else if (z[j] > uX_[j]) {
                            z[j] = uX_[j];
                        }
                    }
                    Real val = P.value(z);
                    if(!std::isnan(val))
					{
						//Accept new point
                        x = z;
                        values_[index].first = val;
                        //mark best
                        if (val < bestValue) {
                            bestValue = val;
                            bestX = x;
                            iterationStat = 0;
                        }
					}
                }
            }
        } while (true);
        if (iteration > maxIteration)
            ecType = EndCriteria::MaxIterations;
        else
            ecType = EndCriteria::StationaryPoint;

        //Set result to best point
        P.setCurrentValue(bestX);
        P.setFunctionValue(bestValue);
        return ecType;
    }

    void FireflyAlgorithm::Intensity::findBrightest() {
        //Brightest ignores all others
        Array& xI = (*xI_)[(*values_)[0].second];
        for (Size j = 0; j < N_; j++) {
            xI[j] = 0.0;
        }

        for (Size i = 1; i < Mfa_; i++) {
            //values_ is already sorted
            Size index = (*values_)[i].second;
            const Array& x = (*x_)[index];
            Array& xI = (*xI_)[index];
            for (Size j = 0; j < N_; j++) {
                xI[j] = 0.0;
            }
            Real valueX = (*values_)[i].first;
            for (Size k = 0; k < i - 1; k++){
                const Array& y = (*x_)[(*values_)[k].second];
                Real valueY = (*values_)[k].first;
                Real intensity = intensityImpl(valueX, valueY, distance(x, y));
                xI += intensity*(y - x);
            }
        }
    }
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Ralph Schreyer
 Copyright (C) 2012 Mateusz Kapturski

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

/*! \file differentialevolution.hpp
    \brief Differential Evolution optimization method
*/

#ifndef quantlib_optimization_differential_evolution_hpp
#define quantlib_optimization_differential_evolution_hpp

#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    //! Differential Evolution configuration object
    /*! The algorithm and strategy names are taken from here:

        Price, K., Storn, R., 1997. Differential Evolution -
        A Simple and Efficient Heuristic for Global Optimization
        over Continuous Spaces.
        Journal of Global Optimization, Kluwer Academic Publishers,
        1997, Vol. 11, pp. 341 - 359.

        There are seven basic strategies for creating mutant population
        currently implemented. Three basic crossover types are also
        available.

        Future development:
        1) base element type to be extracted
        2) L differences to be used instead of fixed number
        3) various weights distributions for the differences (dither etc.)
        4) printFullInfo parameter usage to track the algorithm

        \warning This was reported to fail tests on Mac OS X 10.8.4.
    */


    //! %OptimizationMethod using Differential Evolution algorithm
    /*! \ingroup optimizers */
    class DifferentialEvolution: public OptimizationMethod {
      public:
        enum Strategy {
            Rand1Standard,
            BestMemberWithJitter,
            CurrentToBest2Diffs,
            Rand1DiffWithPerVectorDither,
            Rand1DiffWithDither,
            EitherOrWithOptimalRecombination,
            Rand1SelfadaptiveWithRotation
        };
        enum CrossoverType {
            Normal,
            Binomial,
            Exponential
        };

        struct Candidate {
            Array values;
            Real cost = 0.0;
            Candidate(Size size = 0) : values(size, 0.0) {}
        };

        class Configuration {
          public:
            Strategy strategy = BestMemberWithJitter;
            CrossoverType crossoverType = Normal;
            Size populationMembers = 100;
            Real stepsizeWeight = 0.2, crossoverProbability = 0.9;
            unsigned long seed = 0;
            bool applyBounds = true, crossoverIsAdaptive = false;
            std::vector<Array> initialPopulation;
            Array upperBound, lowerBound;

            // Clang seems to have problems if we use '= default' here.
            // NOLINTNEXTLINE(modernize-use-equals-default)
            Configuration() {}

            Configuration& withBounds(bool b = true) {
                applyBounds = b;
                return *this;
            }

            Configuration& withCrossoverProbability(Real p) {
                QL_REQUIRE(p>=0.0 && p<=1.0,
                          "Crossover probability (" << p
                           << ") must be in [0,1] range");
                crossoverProbability = p;
                return *this;
            }

            Configuration& withPopulationMembers(Size n) {
                QL_REQUIRE(n>0, "Positive number of population members required");
                populationMembers = n;
                initialPopulation.clear();
                return *this;
            }

            Configuration& withInitialPopulation(const std::vector<Array>& c) {
                initialPopulation = c;
                populationMembers = c.size();
                return *this;
            }

            Configuration& withUpperBound(const Array& u) {
                upperBound = u;
                return *this;
            }
            
            Configuration& withLowerBound(const Array& l) {
                lowerBound = l;
                return *this;
            }

            Configuration& withSeed(unsigned long s) {
                seed = s;
                return *this;
            }

            Configuration& withAdaptiveCrossover(bool b = true) {
                crossoverIsAdaptive = b;
                return *this;
            }

            Configuration& withStepsizeWeight(Real w) {
                QL_ENSURE(w>=0 && w<=2.0,
                          "Step size weight ("<< w
                          << ") must be in [0,2] range");
                stepsizeWeight = w;
                return *this;
            }

            Configuration& withCrossoverType(CrossoverType t) {
                crossoverType = t;
                return *this;
            }

            Configuration& withStrategy(Strategy s) {
                strategy = s;
                return *this;
            }
        };


        DifferentialEvolution(const Configuration& configuration = Configuration())
        : configuration_(configuration), rng_(configuration.seed) {}

        EndCriteria::Type minimize(Problem& p, const EndCriteria& endCriteria) override;

        const Configuration& configuration() const {
            return configuration_;
        }

      private:
        Configuration configuration_;
        Array upperBound_, lowerBound_;
        mutable Array currGenSizeWeights_, currGenCrossover_;
        Candidate bestMemberEver_;
        MersenneTwisterUniformRng rng_;

        void fillInitialPopulation(std::vector<Candidate>& population,
                                   const Problem& p) const;

        void getCrossoverMask(std::vector<Array>& crossoverMask,
                              std::vector<Array>& invCrossoverMask,
                              const Array& mutationProbabilities) const;

        Array getMutationProbabilities(
                              const std::vector<Candidate>& population) const;

        void adaptSizeWeights() const;

        void adaptCrossover() const;

        void calculateNextGeneration(std::vector<Candidate>& population,
                                     Problem& costFunction) const;

        Array rotateArray(Array inputArray) const;

        void crossover(const std::vector<Candidate>& oldPopulation,
                       std::vector<Candidate> & population,
                       const std::vector<Candidate>& mutantPopulation,
                       const std::vector<Candidate>& mirrorPopulation,
                       Problem& costFunction) const;
    };

}

#endif

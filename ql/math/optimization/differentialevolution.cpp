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

#include <ql/math/optimization/differentialevolution.hpp>
#include <algorithm>
#include <cmath>

namespace QuantLib {

    namespace {

        struct sort_by_cost {
            bool operator()(const DifferentialEvolution::Candidate& left,
                            const DifferentialEvolution::Candidate& right) {
                return left.cost < right.cost;
            }
        };

        template <class I>
        void randomize(I begin, I end,
                       const MersenneTwisterUniformRng& rng) {
            Size n = static_cast<Size>(end-begin);
            for (Size i=n-1; i>0; --i) {
                std::swap(begin[i], begin[rng.nextInt32() % (i+1)]);
            }
        }

    }

    EndCriteria::Type DifferentialEvolution::minimize(Problem& p, const EndCriteria& endCriteria) {
        EndCriteria::Type ecType;
        p.reset();

        if (configuration().upperBound.empty()) {
            upperBound_ = p.constraint().upperBound(p.currentValue());
        } else {
            QL_REQUIRE(configuration().upperBound.size() == p.currentValue().size(),
                       "wrong upper bound size in differential evolution configuration");
            upperBound_ = configuration().upperBound;
        }
        if (configuration().lowerBound.empty()) {
            lowerBound_ = p.constraint().lowerBound(p.currentValue());
        } else {
            QL_REQUIRE(configuration().lowerBound.size() == p.currentValue().size(),
                       "wrong lower bound size in differential evolution configuration");
            lowerBound_ = configuration().lowerBound;
        }
        currGenSizeWeights_ =
            Array(configuration().populationMembers, configuration().stepsizeWeight);
        currGenCrossover_ = Array(configuration().populationMembers,
                                  configuration().crossoverProbability);

        std::vector<Candidate> population;
        if (!configuration().initialPopulation.empty()) {
            population.resize(configuration().initialPopulation.size());
            for (Size i = 0; i < population.size(); ++i) {
                population[i].values = configuration().initialPopulation[i];
                QL_REQUIRE(population[i].values.size() == p.currentValue().size(),
                           "wrong values size in initial population");
                population[i].cost = p.costFunction().value(population[i].values);
            }
        } else {
            population = std::vector<Candidate>(configuration().populationMembers,
                                                Candidate(p.currentValue().size()));
            fillInitialPopulation(population, p);
        }

        std::partial_sort(population.begin(), population.begin() + 1, population.end(),
                          sort_by_cost());
        bestMemberEver_ = population.front();
        Real fxOld = population.front().cost;
        Size iteration = 0, stationaryPointIteration = 0;

        // main loop - calculate consecutive emerging populations
        while (!endCriteria.checkMaxIterations(iteration++, ecType)) {
            calculateNextGeneration(population, p);
            std::partial_sort(population.begin(), population.begin() + 1, population.end(),
                              sort_by_cost());
            if (population.front().cost < bestMemberEver_.cost)
                bestMemberEver_ = population.front();
            Real fxNew = population.front().cost;
            if (endCriteria.checkStationaryFunctionValue(fxOld, fxNew, stationaryPointIteration,
                                                         ecType))
                break;
            fxOld = fxNew;
        };
        p.setCurrentValue(bestMemberEver_.values);
        p.setFunctionValue(bestMemberEver_.cost);
        return ecType;
    }

    void DifferentialEvolution::calculateNextGeneration(
                                     std::vector<Candidate>& population,
                                     Problem& p) const {

        std::vector<Candidate> mirrorPopulation;
        std::vector<Candidate> oldPopulation = population;

        switch (configuration().strategy) {

          case Rand1Standard: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop2 = population;
              randomize(population.begin(), population.end(), rng_);
              mirrorPopulation = shuffledPop1;

              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  population[popIter].values = population[popIter].values
                      + configuration().stepsizeWeight
                      * (shuffledPop1[popIter].values - shuffledPop2[popIter].values);
              }
          }
            break;

          case BestMemberWithJitter: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              Array jitter(population[0].values.size(), 0.0);

              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  for (double& jitterIter : jitter) {
                      jitterIter = rng_.nextReal();
                  }
                  population[popIter].values = bestMemberEver_.values
                      + (shuffledPop1[popIter].values - population[popIter].values)
                      * (0.0001 * jitter + configuration().stepsizeWeight);
              }
              mirrorPopulation = std::vector<Candidate>(population.size(),
                                                        bestMemberEver_);
          }
            break;

          case CurrentToBest2Diffs: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);

              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  population[popIter].values = oldPopulation[popIter].values
                      + configuration().stepsizeWeight
                      * (bestMemberEver_.values - oldPopulation[popIter].values)
                      + configuration().stepsizeWeight
                      * (population[popIter].values - shuffledPop1[popIter].values);
              }
              mirrorPopulation = shuffledPop1;
          }
            break;

          case Rand1DiffWithPerVectorDither: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop2 = population;
              randomize(population.begin(), population.end(), rng_);
              mirrorPopulation = shuffledPop1;
              Array FWeight = Array(population.front().values.size(), 0.0);
              for (double& fwIter : FWeight)
                  fwIter = (1.0 - configuration().stepsizeWeight) * rng_.nextReal() +
                           configuration().stepsizeWeight;
              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  population[popIter].values = population[popIter].values
                      + FWeight * (shuffledPop1[popIter].values - shuffledPop2[popIter].values);
              }
          }
            break;

          case Rand1DiffWithDither: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop2 = population;
              randomize(population.begin(), population.end(), rng_);
              mirrorPopulation = shuffledPop1;
              Real FWeight = (1.0 - configuration().stepsizeWeight) * rng_.nextReal()
                  + configuration().stepsizeWeight;
              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  population[popIter].values = population[popIter].values
                      + FWeight * (shuffledPop1[popIter].values - shuffledPop2[popIter].values);
              }
          }
            break;

          case EitherOrWithOptimalRecombination: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop2 = population;
              randomize(population.begin(), population.end(), rng_);
              mirrorPopulation = shuffledPop1;
              Real probFWeight = 0.5;
              if (rng_.nextReal() < probFWeight) {
                  for (Size popIter = 0; popIter < population.size(); popIter++) {
                      population[popIter].values = oldPopulation[popIter].values
                          + configuration().stepsizeWeight
                          * (shuffledPop1[popIter].values - shuffledPop2[popIter].values);
                  }
              } else {
                  Real K = 0.5 * (configuration().stepsizeWeight + 1); // invariant with respect to probFWeight used
                  for (Size popIter = 0; popIter < population.size(); popIter++) {
                      population[popIter].values = oldPopulation[popIter].values
                          + K
                          * (shuffledPop1[popIter].values - shuffledPop2[popIter].values
                             - 2.0 * population[popIter].values);
                  }
              }
          }
            break;

          case Rand1SelfadaptiveWithRotation: {
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop1 = population;
              randomize(population.begin(), population.end(), rng_);
              std::vector<Candidate> shuffledPop2 = population;
              randomize(population.begin(), population.end(), rng_);
              mirrorPopulation = shuffledPop1;

              adaptSizeWeights();

              for (Size popIter = 0; popIter < population.size(); popIter++) {
                  if (rng_.nextReal() < 0.1){
                      population[popIter].values = rotateArray(bestMemberEver_.values);
                  }else {
                      population[popIter].values = bestMemberEver_.values
                          + currGenSizeWeights_[popIter]
                          * (shuffledPop1[popIter].values - shuffledPop2[popIter].values);
                  }
              }
          }
            break;

          default:
            QL_FAIL("Unknown strategy ("
                    << Integer(configuration().strategy) << ")");
        }
        // in order to avoid unnecessary copying we use the same population object for mutants
        crossover(oldPopulation, population, population, mirrorPopulation, p);
    }

    void DifferentialEvolution::crossover(
                               const std::vector<Candidate>& oldPopulation,
                               std::vector<Candidate>& population,
                               const std::vector<Candidate>& mutantPopulation,
                               const std::vector<Candidate>& mirrorPopulation,
                               Problem& p) const {

        if (configuration().crossoverIsAdaptive) {
            adaptCrossover();
        }

        Array mutationProbabilities = getMutationProbabilities(population);

        std::vector<Array> crossoverMask(population.size(),
                                         Array(population.front().values.size(), 1.0));
        std::vector<Array> invCrossoverMask = crossoverMask;
        getCrossoverMask(crossoverMask, invCrossoverMask, mutationProbabilities);

        // crossover of the old and mutant population
        for (Size popIter = 0; popIter < population.size(); popIter++) {
            population[popIter].values = oldPopulation[popIter].values * invCrossoverMask[popIter]
                + mutantPopulation[popIter].values * crossoverMask[popIter];
            // immediately apply bounds if specified
            if (configuration().applyBounds) {
                for (Size memIter = 0; memIter < population[popIter].values.size(); memIter++) {
                    if (population[popIter].values[memIter] > upperBound_[memIter])
                        population[popIter].values[memIter] = upperBound_[memIter]
                            + rng_.nextReal()
                            * (mirrorPopulation[popIter].values[memIter]
                               - upperBound_[memIter]);
                    if (population[popIter].values[memIter] < lowerBound_[memIter])
                        population[popIter].values[memIter] = lowerBound_[memIter]
                            + rng_.nextReal()
                            * (mirrorPopulation[popIter].values[memIter]
                               - lowerBound_[memIter]);
                }
            }
            // evaluate objective function as soon as possible to avoid unnecessary loops
            try {
                population[popIter].cost = p.value(population[popIter].values);
            } catch (Error&) {
                population[popIter].cost = QL_MAX_REAL;
            }
            if (!std::isfinite(population[popIter].cost))
                population[popIter].cost = QL_MAX_REAL;

        }
    }

    void DifferentialEvolution::getCrossoverMask(
                                  std::vector<Array> & crossoverMask,
                                  std::vector<Array> & invCrossoverMask,
                                  const Array & mutationProbabilities) const {
        for (Size cmIter = 0; cmIter < crossoverMask.size(); cmIter++) {
            for (Size memIter = 0; memIter < crossoverMask[cmIter].size(); memIter++) {
                if (rng_.nextReal() < mutationProbabilities[cmIter]) {
                    invCrossoverMask[cmIter][memIter] = 0.0;
                } else {
                    crossoverMask[cmIter][memIter] = 0.0;
                }
            }
        }
    }

    Array DifferentialEvolution::getMutationProbabilities(
                            const std::vector<Candidate> & population) const {
        Array mutationProbabilities = currGenCrossover_;
        switch (configuration().crossoverType) {
          case Normal:
            break;
          case Binomial:
            mutationProbabilities = currGenCrossover_
                * (1.0 - 1.0 / population.front().values.size())
                + 1.0 / population.front().values.size();
            break;
          case Exponential:
            for (Size coIter = 0;coIter< currGenCrossover_.size(); coIter++){
                mutationProbabilities[coIter] =
                    (1.0 - std::pow(currGenCrossover_[coIter],
                                    (int) population.front().values.size()))
                    / (population.front().values.size()
                       * (1.0 - currGenCrossover_[coIter]));
            }
            break;
          default:
            QL_FAIL("Unknown crossover type ("
                    << Integer(configuration().crossoverType) << ")");
            break;
        }
        return mutationProbabilities;
    }

    Array DifferentialEvolution::rotateArray(Array a) const {
        randomize(a.begin(), a.end(), rng_);
        return a;
    }

    void DifferentialEvolution::adaptSizeWeights() const {
        // [=Fl & =Fu] respectively see Brest, J. et al., 2006,
        // "Self-Adapting Control Parameters in Differential
        // Evolution"
        Real sizeWeightLowerBound = 0.1, sizeWeightUpperBound = 0.9;
         // [=tau1] A Comparative Study on Numerical Benchmark
         // Problems." page 649 for reference
        Real sizeWeightChangeProb = 0.1;
        for (double& currGenSizeWeight : currGenSizeWeights_) {
            if (rng_.nextReal() < sizeWeightChangeProb)
                currGenSizeWeight = sizeWeightLowerBound + rng_.nextReal() * sizeWeightUpperBound;
        }
    }

    void DifferentialEvolution::adaptCrossover() const {
        Real crossoverChangeProb = 0.1; // [=tau2]
        for (double& coIter : currGenCrossover_) {
            if (rng_.nextReal() < crossoverChangeProb)
                coIter = rng_.nextReal();
        }
    }

    void DifferentialEvolution::fillInitialPopulation(
                                          std::vector<Candidate> & population,
                                          const Problem& p) const {

        // use initial values provided by the user
        population.front().values = p.currentValue();
        population.front().cost = p.costFunction().value(population.front().values);
        // rest of the initial population is random
        for (Size j = 1; j < population.size(); ++j) {
            for (Size i = 0; i < p.currentValue().size(); ++i) {
                Real l = lowerBound_[i], u = upperBound_[i];
                population[j].values[i] = l + (u-l)*rng_.nextReal();
            }
            population[j].cost = p.costFunction().value(population[j].values);
            if (!std::isfinite(population[j].cost))
                population[j].cost = QL_MAX_REAL;
        }
    }

}

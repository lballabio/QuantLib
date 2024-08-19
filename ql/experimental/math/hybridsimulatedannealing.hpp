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

/*! \file hybridsimulatedannealing.hpp
\brief Implementation based on:
Very Fast Simulated Re-Annealing, Lester Ingber,
Mathl. Comput. Modelling, 967-973, 1989
*/

#ifndef quantlib_optimization_hybridsimulatedannealing_hpp
#define quantlib_optimization_hybridsimulatedannealing_hpp

#include <ql/experimental/math/hybridsimulatedannealingfunctors.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/shared_ptr.hpp>
#include <utility>

namespace QuantLib {

    /*! Method is fairly straightforward:
    1) Sampler provides a probability density (based on current value) for the parameters. Each
    iteration a new draw is made from it to find a new point
    2) Probability determines whether the new point, obtained from Sampler, is accepted or not
    3) Temperature is a schedule T(k) for the iteration k, which affects the Sampler and Probability
    4) Reannealing is a departure from the traditional Boltzmann Annealing method: it rescales
    the iteration k independently for each dimension so as to improve convergence

    The hybrid in the name is because one can provide it a local optimizer for use whenever any new
    best point is found or at every accepted point, in which case is used is chose by the user.

    Class Sampler must implement the following interface:
    \code
    void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const;
    \endcode
    Class Probability must implement the following interface:
    \code
    bool operator()(Real currentValue, Real newValue, const Array &temp) const;
    \endcode
    Class Temperature must implement the following interface:
    \code
    void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const;
    \endcode
    Class Reannealing must implement the following interface:
    \code
    void operator()(Array & steps, const Array &currentPoint,
    Real aCurrentValue, const Array & currTemp) const;
    \endcode
    */
    template <class Sampler, class Probability, class Temperature, class Reannealing = ReannealingTrivial>
    class HybridSimulatedAnnealing : public OptimizationMethod {
      public:
        enum LocalOptimizeScheme {
            NoLocalOptimize,
            EveryNewPoint,
            EveryBestPoint
        };
        enum ResetScheme {
            NoResetScheme,
            ResetToBestPoint,
            ResetToOrigin
        };

        HybridSimulatedAnnealing(const Sampler& sampler,
                                 const Probability& probability,
                                 Temperature temperature,
                                 const Reannealing& reannealing = ReannealingTrivial(),
                                 Real startTemperature = 200.0,
                                 Real endTemperature = 0.01,
                                 Size reAnnealSteps = 50,
                                 ResetScheme resetScheme = ResetToBestPoint,
                                 Size resetSteps = 150,
                                 ext::shared_ptr<OptimizationMethod> localOptimizer =
                                     ext::shared_ptr<OptimizationMethod>(),
                                 LocalOptimizeScheme optimizeScheme = EveryBestPoint)
        : sampler_(sampler), probability_(probability), temperature_(std::move(temperature)),
          reannealing_(reannealing), startTemperature_(startTemperature),
          endTemperature_(endTemperature),
          reAnnealSteps_(reAnnealSteps == 0 ? QL_MAX_INTEGER : reAnnealSteps),
          resetScheme_(resetScheme), resetSteps_(resetSteps == 0 ? QL_MAX_INTEGER : resetSteps),
          localOptimizer_(localOptimizer),
          optimizeScheme_(localOptimizer != nullptr ? optimizeScheme : NoLocalOptimize) {
            if (!localOptimizer)
                localOptimizer = ext::make_shared<LevenbergMarquardt>();
        }

        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;

      private:
        Sampler sampler_;
        Probability probability_;
        Temperature temperature_;
        Reannealing reannealing_;
        Real startTemperature_;
        Real endTemperature_;
        Size reAnnealSteps_;
        ResetScheme resetScheme_;
        Size resetSteps_;
        ext::shared_ptr<OptimizationMethod> localOptimizer_;
        LocalOptimizeScheme optimizeScheme_;
    };

    template <class Sampler, class Probability, class Temperature, class Reannealing>
    EndCriteria::Type HybridSimulatedAnnealing<Sampler, Probability, Temperature, Reannealing>::minimize(Problem &P, const EndCriteria &endCriteria) {
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        reannealing_.setProblem(P);
        Array x = P.currentValue();
        Size n = x.size();
        Size k = 1;
        Size kStationary = 1;
        Size kReAnneal = 1;
        Size kReset = 1;
        Size maxK = endCriteria.maxIterations();
        Size maxKStationary = endCriteria.maxStationaryStateIterations();
        bool temperatureBreached = false;
        Array currentTemperature(n, startTemperature_);
        Array annealStep(n, 1.0);
        Array bestPoint(x);
        Array currentPoint(x);
        const Array& startingPoint(x);
        Array newPoint(x);
        Real bestValue = P.value(bestPoint);
        Real currentValue = bestValue;
        Real startingValue = bestValue; //to reset to starting point if desired
        while (k <= maxK && kStationary <= maxKStationary && !temperatureBreached)
        {
            //Draw a new sample point
            sampler_(newPoint, currentPoint, currentTemperature);
            try{
                //Evaluate new point
                Real newValue = P.value(newPoint);
				
                //Determine if new point is accepted
                if (probability_(currentValue, newValue, currentTemperature)) {
                    if (optimizeScheme_ == EveryNewPoint) {
                        P.setCurrentValue(newPoint);
                        P.setFunctionValue(newValue);
                        localOptimizer_->minimize(P, endCriteria);
                        newPoint = P.currentValue();
                        newValue = P.functionValue();
                    }
                    currentPoint = newPoint;
                    currentValue = newValue;
                }

                //Check if we have a new best point
                if (newValue < bestValue) {
                    if (optimizeScheme_ == EveryBestPoint) {
                        P.setCurrentValue(newPoint);
                        P.setFunctionValue(newValue);
                        localOptimizer_->minimize(P, endCriteria);
                        newPoint = P.currentValue();
                        newValue = P.functionValue();
                    }
                    kStationary = 0;
                    bestValue = newValue;
                    bestPoint = newPoint;
                }
            } catch(...){
                //Do nothing, move on to new draw
            }
            //Increase steps
            k++;
            kStationary++;
            for (Real& i : annealStep)
                i++;

            //Reanneal if necessary
            if (kReAnneal == reAnnealSteps_) {
                kReAnneal = 0;
                reannealing_(annealStep, currentPoint, currentValue, currentTemperature);
            }
            kReAnneal++;

            //Reset if necessary
            if (kReset == resetSteps_) {
                kReset = 0;
                switch (resetScheme_) {
                case NoResetScheme:
                    break;
                case ResetToOrigin:
                    currentPoint = startingPoint;
                    currentValue = startingValue;
                    break;
                case ResetToBestPoint:
                    currentPoint = bestPoint;
                    currentValue = bestValue;
                    break;
                }
            }
            kReset++;

            //Update the current temperature according to current step
            temperature_(currentTemperature, currentTemperature, annealStep);

            //Check if temperature condition is breached
            for (Size i = 0; i < n; i++)
                temperatureBreached = temperatureBreached && currentTemperature[i] < endTemperature_;
        }
        
        //Change end criteria type if appropriate
        if (k > maxK)
            ecType = EndCriteria::MaxIterations;
        else if (kStationary > maxKStationary)
            ecType = EndCriteria::StationaryPoint;

        //Set result to best point
        P.setCurrentValue(bestPoint);
        P.setFunctionValue(bestValue);
        return ecType;
    }

    typedef HybridSimulatedAnnealing<SamplerGaussian, ProbabilityBoltzmannDownhill, TemperatureExponential, ReannealingTrivial> GaussianSimulatedAnnealing;
    typedef HybridSimulatedAnnealing<SamplerLogNormal, ProbabilityBoltzmannDownhill, TemperatureExponential, ReannealingTrivial> LogNormalSimulatedAnnealing;
    typedef HybridSimulatedAnnealing<SamplerMirrorGaussian, ProbabilityBoltzmannDownhill, TemperatureExponential, ReannealingTrivial> MirrorGaussianSimulatedAnnealing;
    typedef HybridSimulatedAnnealing<SamplerGaussian, ProbabilityBoltzmannDownhill, TemperatureExponential, ReannealingFiniteDifferences> GaussianSimulatedReAnnealing;
    typedef HybridSimulatedAnnealing<SamplerVeryFastAnnealing, ProbabilityBoltzmannDownhill, TemperatureVeryFastAnnealing, ReannealingTrivial> VeryFastSimulatedAnnealing;
    typedef HybridSimulatedAnnealing<SamplerVeryFastAnnealing, ProbabilityBoltzmannDownhill, TemperatureVeryFastAnnealing, ReannealingFiniteDifferences> VeryFastSimulatedReAnnealing;
}

#endif

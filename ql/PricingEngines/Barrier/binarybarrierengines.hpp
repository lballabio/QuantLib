
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binarybarrierengines.hpp
    \brief binary barrier option engines

    Based on the Vanilla Engine pattern
*/

#ifndef quantlib_binarybarrier_engines_h
#define quantlib_binarybarrier_engines_h

#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>
#include <ql/MonteCarlo/binarybarrierpathpricer.hpp>
#include <ql/MonteCarlo/mctraits.hpp>
#include <ql/PricingEngines/Vanilla/mcvanillaengine.hpp>

namespace QuantLib {

    //! Binary engine base class
    class BinaryBarrierEngine
        : public GenericEngine<BinaryBarrierOption::arguments,
                               BinaryBarrierOption::results> {};

    //! Analytic pricing engine for European binary barrier options
    class AnalyticEuropeanBinaryBarrierEngine : public BinaryBarrierEngine {
      public:
        void calculate() const;
    };

    //! Analytic pricing engine for American binary barrier options formulae
    class AnalyticAmericanBinaryBarrierEngine : public BinaryBarrierEngine {
      public:
        void calculate() const;
    };

    //! Pricing engine for binary barrier options using Monte Carlo simulation
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCBinaryBarrierEngine : public BinaryBarrierEngine,
                           public McSimulation<SingleAsset<RNG>, S> {
      public:
        // constructor
        MCBinaryBarrierEngine(Size maxTimeStepsPerYear,
                       bool antitheticVariate = false,
                       bool controlVariate = false,
                       Size requiredSamples = Null<int>(),
                       double requiredTolerance = Null<double>(),
                       Size maxSamples = Null<int>(),
                       long seed = 0);

        void calculate() const;
      protected:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;

        // the uniform generator to use in path generation and
        // path correction
        //typedef typename RNG::ursg_type ursg_type;
        typedef typename RNG::ursg_type my_sequence_type;

        // McSimulation implementation
        Handle<path_generator_type> pathGenerator() const;
        TimeGrid timeGrid() const;
        Handle<path_pricer_type> pathPricer() const;

        // data members
        //my_sequence_type uniformGenerator_;
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        double requiredTolerance_;
        long seed_;
    };


    template<class RNG, class S>
    MCBinaryBarrierEngine<RNG,S>::MCBinaryBarrierEngine(
                                                     Size maxTimeStepsPerYear,
                                                     bool antitheticVariate,
                                                     bool controlVariate,
                                                     Size requiredSamples,
                                                     double requiredTolerance,
                                                     Size maxSamples,
                                                     long seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate,
                                       controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples),
      maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      seed_(seed) {}


    template<class RNG, class S>
    Handle<QL_TYPENAME MCBinaryBarrierEngine<RNG,S>::path_generator_type>
    MCBinaryBarrierEngine<RNG,S>::pathGenerator() const {

        Handle<DiffusionProcess> bs(new
            BlackScholesProcess(
                arguments_.blackScholesProcess->riskFreeTS,
                arguments_.blackScholesProcess->dividendTS,
                arguments_.blackScholesProcess->volTS,
                arguments_.blackScholesProcess->stateVariable->value()));

        TimeGrid grid = timeGrid();

        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1, seed_);
        // BB here
        return Handle<path_generator_type>(new
            path_generator_type(bs, grid, gen, false));

    }


    template <class RNG, class S>
    Handle<QL_TYPENAME MCBinaryBarrierEngine<RNG,S>::path_pricer_type>
    MCBinaryBarrierEngine<RNG,S>::pathPricer() const {

        #if defined(HAVE_BOOST)
        Handle<CashOrNothingPayoff> payoff =
            boost::dynamic_pointer_cast<CashOrNothingPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "MCBinaryBarrierEngine: wrong payoff given");
        #else
        Handle<CashOrNothingPayoff> payoff = arguments_.payoff;
        #endif


        #if defined(HAVE_BOOST)
        Handle<AmericanExercise> exercise =
            boost::dynamic_pointer_cast<AmericanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise,
                   "MCBinaryBarrierEngine: wrong exercise given");
        #else
        Handle<AmericanExercise> exercise = arguments_.exercise;
        #endif

        TimeGrid grid = timeGrid();
        UniformRandomSequenceGenerator
            sequenceGen(grid.size()-1, UniformRandomGenerator(76));

        return Handle<MCBinaryBarrierEngine<RNG,S>::path_pricer_type>(new
          BinaryBarrierPathPricer(
            payoff,
            exercise,
            arguments_.blackScholesProcess->stateVariable->value(),
            arguments_.blackScholesProcess->riskFreeTS,
            Handle<DiffusionProcess>(new
              BlackScholesProcess(
                  arguments_.blackScholesProcess->riskFreeTS,
                  arguments_.blackScholesProcess->dividendTS,
                  arguments_.blackScholesProcess->volTS,
                  arguments_.blackScholesProcess->stateVariable->value())),
            sequenceGen));
    }


    template <class RNG, class S>
    inline
    TimeGrid MCBinaryBarrierEngine<RNG,S>::timeGrid() const {
        Time t =
          arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
          arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
          arguments_.exercise->lastDate());
        return TimeGrid(t, Size(QL_MAX(t * maxTimeStepsPerYear_, 1.0)));
    }

    template<class RNG, class S>
    void MCBinaryBarrierEngine<RNG,S>::calculate() const {

        QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                   int(requiredSamples_) != Null<int>(),
                   "MCBinaryBarrierEngine::calculate: "
                   "neither tolerance nor number of samples set");

        //! Initialize the one-factor Monte Carlo
        if (controlVariate_) {

            Handle<path_pricer_type> controlPP = controlPathPricer();
            QL_REQUIRE(!IsNull(controlPP),
                       "MCBinaryBarrierEngine::calculate() : "
                       "engine does not provide "
                       "control variation path pricer");

            Handle<PricingEngine> controlPE = controlPricingEngine();

            QL_REQUIRE(!IsNull(controlPE),
                       "MCBinaryBarrierEngine::calculate() : "
                       "engine does not provide "
                       "control variation pricing engine");
        } else {
            mcModel_ =
                Handle<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                        pathGenerator(), pathPricer(), S(),
                        antitheticVariate_));
        }

        if (requiredTolerance_ != Null<double>()) {
            if (int(maxSamples_) != Null<int>())
                value(requiredTolerance_, maxSamples_);
            else
                value(requiredTolerance_);
        } else {
            valueWithSamples(requiredSamples_);
        }

        results_.value = mcModel_->sampleAccumulator().mean();
        if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                mcModel_->sampleAccumulator().errorEstimate();
    }

}


#endif

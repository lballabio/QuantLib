/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

/*! \file mclookbackpartialfixedengine.hpp
    \brief Monte Carlo lookback partial fixed engine
*/

#ifndef quantlib_mc_lookback_partial_fixed_engines_hpp
#define quantlib_mc_lookback_partial_fixed_engines_hpp

#include <ql/exercise.hpp>
#include <ql/instruments/lookbackoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCLookbackPartialFixedEngine : public ContinuousPartialFixedLookbackOption::engine,
                                         public McSimulation<SingleVariate,RNG,S> {
      public:
        typedef typename McSimulation<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        // constructor
        MCLookbackPartialFixedEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool brownianBridge,
             bool antithetic,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
        void calculate() const {
            Real spot = process_->x0();
            QL_REQUIRE(spot >= 0.0, "negative or null underlying given");
            McSimulation<SingleVariate,RNG,S>::calculate(requiredTolerance_,
                                                         requiredSamples_,
                                                         maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        // McSimulation implementation
        TimeGrid timeGrid() const;
        ext::shared_ptr<path_generator_type> pathGenerator() const {
            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(process_,
                                                 grid, gen, brownianBridge_));
        }
        ext::shared_ptr<path_pricer_type> pathPricer() const;
        // data members
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool antithetic_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    //! Monte Carlo lookback-option engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCLookbackPartialFixedEngine {
      public:
        explicit MakeMCLookbackPartialFixedEngine(
                    const ext::shared_ptr<GeneralizedBlackScholesProcess>&);
        // named parameters
        MakeMCLookbackPartialFixedEngine& withSteps(Size steps);
        MakeMCLookbackPartialFixedEngine& withStepsPerYear(Size steps);
        MakeMCLookbackPartialFixedEngine& withBrownianBridge(bool b = true);
        MakeMCLookbackPartialFixedEngine& withAntitheticVariate(bool b = true);
        MakeMCLookbackPartialFixedEngine& withSamples(Size samples);
        MakeMCLookbackPartialFixedEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCLookbackPartialFixedEngine& withMaxSamples(Size samples);
        MakeMCLookbackPartialFixedEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool brownianBridge_, antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };

    class LookbackPartialFixedPathPricer : public PathPricer<Path> {
      public:
        LookbackPartialFixedPathPricer(Time lookbackStart,
                    Option::Type type,
                    Real strike,
                    const DiscountFactor discount);
        Real operator()(const Path& path) const;
      private:
        Time lookbackStart_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };

    // template definitions

    template <class RNG, class S>
    inline MCLookbackPartialFixedEngine<RNG,S>::MCLookbackPartialFixedEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : McSimulation<SingleVariate,RNG,S>(antitheticVariate, false),
      process_(process), timeSteps_(timeSteps),
      timeStepsPerYear_(timeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {
        QL_REQUIRE(timeSteps != Null<Size>() ||
                   timeStepsPerYear != Null<Size>(),
                   "no time steps provided");
        QL_REQUIRE(timeSteps == Null<Size>() ||
                   timeStepsPerYear == Null<Size>(),
                   "both time steps and time steps per year were provided");
        QL_REQUIRE(timeSteps != 0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
        QL_REQUIRE(timeStepsPerYear != 0,
                   "timeStepsPerYear must be positive, " << timeStepsPerYear <<
                   " not allowed");
        registerWith(process_);
    }

    template <class RNG, class S>
    inline TimeGrid MCLookbackPartialFixedEngine<RNG,S>::timeGrid() const {

        Time residualTime = process_->time(arguments_.exercise->lastDate());
        if (timeSteps_ != Null<Size>()) {
            return TimeGrid(residualTime, timeSteps_);
        } else if (timeStepsPerYear_ != Null<Size>()) {
            Size steps = static_cast<Size>(timeStepsPerYear_*residualTime);
            return TimeGrid(residualTime, std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCLookbackPartialFixedEngine<RNG,S>::path_pricer_type>
    MCLookbackPartialFixedEngine<RNG,S>::pathPricer() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        TimeGrid grid = timeGrid();
        DiscountFactor discount = process_->riskFreeRate()->discount(grid.back());

        Time lookbackStart = process_->time(arguments_.lookbackPeriodStart);

        return ext::shared_ptr<
                    typename MCLookbackPartialFixedEngine<RNG,S>::path_pricer_type>(
            new LookbackPartialFixedPathPricer(
                lookbackStart,
                payoff->optionType(),
                payoff->strike(),
                discount));
        }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>::MakeMCLookbackPartialFixedEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process),
      brownianBridge_(false), antithetic_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>&
    MakeMCLookbackPartialFixedEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCLookbackPartialFixedEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");

        return ext::shared_ptr<PricingEngine>(new MCLookbackPartialFixedEngine<RNG,S>(process_,
                                   steps_,
                                   stepsPerYear_,
                                   brownianBridge_,
                                   antithetic_,
                                   samples_,
                                   tolerance_,
                                   maxSamples_,
                                   seed_));
    }

}


#endif

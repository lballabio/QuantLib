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

/*! \file mcdoublebarrierengine.hpp
    \brief Monte Carlo barrier option engines
*/

#ifndef quantlib_mc_double_barrier_engines_hpp
#define quantlib_mc_double_barrier_engines_hpp

#include <ql/exercise.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDoubleBarrierEngine : public DoubleBarrierOption::engine,
                                  public McSimulation<SingleVariate,RNG,S> {
      public:
        typedef typename McSimulation<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        // constructor
        MCDoubleBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                              Size timeSteps,
                              Size timeStepsPerYear,
                              bool brownianBridge,
                              bool antithetic,
                              Size requiredSamples,
                              Real requiredTolerance,
                              Size maxSamples,
                              BigNatural seed);
        void calculate() const override {
            Real spot = process_->x0();
            QL_REQUIRE(spot >= 0.0, "negative or null underlying given");
            QL_REQUIRE(!triggered(spot), "barrier touched");
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
        TimeGrid timeGrid() const override;
        ext::shared_ptr<path_generator_type> pathGenerator() const override {
            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(process_,
                                                 grid, gen, brownianBridge_));
        }
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
        // data members
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool antithetic_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    //! Monte Carlo double-barrier-option engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDoubleBarrierEngine {
      public:
        explicit MakeMCDoubleBarrierEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        // named parameters
        MakeMCDoubleBarrierEngine& withSteps(Size steps);
        MakeMCDoubleBarrierEngine& withStepsPerYear(Size steps);
        MakeMCDoubleBarrierEngine& withBrownianBridge(bool b = true);
        MakeMCDoubleBarrierEngine& withAntitheticVariate(bool b = true);
        MakeMCDoubleBarrierEngine& withSamples(Size samples);
        MakeMCDoubleBarrierEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDoubleBarrierEngine& withMaxSamples(Size samples);
        MakeMCDoubleBarrierEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool brownianBridge_, antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };

    class DoubleBarrierPathPricer : public PathPricer<Path> {
      public:
        DoubleBarrierPathPricer(DoubleBarrier::Type barrierType,
                                Real barrierLow,
                                Real barrieHigh,
                                Real rebate,
                                Option::Type type,
                                Real strike,
                                std::vector<DiscountFactor> discounts);
        Real operator()(const Path& path) const override;

      private:
        DoubleBarrier::Type barrierType_;
        Real barrierLow_;
        Real barrierHigh_;
        Real rebate_;
        PlainVanillaPayoff payoff_;
        std::vector<DiscountFactor> discounts_;
    };

    // template definitions

    template <class RNG, class S>
    inline MCDoubleBarrierEngine<RNG, S>::MCDoubleBarrierEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<SingleVariate, RNG, S>(antitheticVariate, false), process_(std::move(process)),
      timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear), requiredSamples_(requiredSamples),
      maxSamples_(maxSamples), requiredTolerance_(requiredTolerance),
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
    inline TimeGrid MCDoubleBarrierEngine<RNG,S>::timeGrid() const {

        Time residualTime = process_->time(arguments_.exercise->lastDate());
        if (timeSteps_ != Null<Size>()) {
            return TimeGrid(residualTime, timeSteps_);
        } else if (timeStepsPerYear_ != Null<Size>()) {
            Size steps = ql_cast<Size>(timeStepsPerYear_*residualTime);
            return TimeGrid(residualTime, std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCDoubleBarrierEngine<RNG,S>::path_pricer_type>
    MCDoubleBarrierEngine<RNG,S>::pathPricer() const {
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        TimeGrid grid = timeGrid();
        std::vector<DiscountFactor> discounts(grid.size());
        for (Size i=0; i<grid.size(); i++)
            discounts[i] = process_->riskFreeRate()->discount(grid[i]);

        return ext::shared_ptr<
                    typename MCDoubleBarrierEngine<RNG,S>::path_pricer_type>(
            new DoubleBarrierPathPricer(
                arguments_.barrierType,
                arguments_.barrier_lo,
                arguments_.barrier_hi,
                arguments_.rebate,
                payoff->optionType(),
                payoff->strike(),
                discounts));
        }

        template <class RNG, class S>
        inline MakeMCDoubleBarrierEngine<RNG, S>::MakeMCDoubleBarrierEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
        : process_(std::move(process)), brownianBridge_(false), antithetic_(false),
          steps_(Null<Size>()), stepsPerYear_(Null<Size>()), samples_(Null<Size>()),
          maxSamples_(Null<Size>()), tolerance_(Null<Real>()), seed_(0) {}

        template <class RNG, class S>
        inline MakeMCDoubleBarrierEngine<RNG, S>&
        MakeMCDoubleBarrierEngine<RNG, S>::withSteps(Size steps) {
            steps_ = steps;
            return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDoubleBarrierEngine<RNG,S>&
    MakeMCDoubleBarrierEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDoubleBarrierEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(new
            MCDoubleBarrierEngine<RNG,S>(process_,
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

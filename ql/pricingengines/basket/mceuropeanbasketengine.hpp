/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file mceuropeanbasketengine.hpp
    \brief European basket MC Engine
*/

#ifndef quantlib_mc_european_basket_engine_hpp
#define quantlib_mc_european_basket_engine_hpp

#include <ql/exercise.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <utility>

namespace QuantLib {

    //! Pricing engine for European basket options using Monte Carlo simulation
    /*! \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCEuropeanBasketEngine  : public BasketOption::engine,
                                    public McSimulation<MultiVariate,RNG,S> {
      public:
        typedef typename McSimulation<MultiVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCEuropeanBasketEngine(std::shared_ptr<StochasticProcessArray>,
                               Size timeSteps,
                               Size timeStepsPerYear,
                               bool brownianBridge,
                               bool antitheticVariate,
                               Size requiredSamples,
                               Real requiredTolerance,
                               Size maxSamples,
                               BigNatural seed);
        void calculate() const override {
            McSimulation<MultiVariate,RNG,S>::calculate(requiredTolerance_,
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
        std::shared_ptr<path_generator_type> pathGenerator() const override {

            std::shared_ptr<BasketPayoff> payoff =
                std::dynamic_pointer_cast<BasketPayoff>(
                                                          arguments_.payoff);
            QL_REQUIRE(payoff, "non-basket payoff given");

            Size numAssets = processes_->size();

            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(numAssets*(grid.size()-1),seed_);

            return std::shared_ptr<path_generator_type>(
                         new path_generator_type(processes_,
                                                 grid, gen, brownianBridge_));
        }
        std::shared_ptr<path_pricer_type> pathPricer() const override;
        // data members
        std::shared_ptr<StochasticProcessArray> processes_;
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_;
        Size maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    //! Monte Carlo basket-option engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEuropeanBasketEngine {
      public:
        MakeMCEuropeanBasketEngine(std::shared_ptr<StochasticProcessArray>);
        // named parameters
        MakeMCEuropeanBasketEngine& withSteps(Size steps);
        MakeMCEuropeanBasketEngine& withStepsPerYear(Size steps);
        MakeMCEuropeanBasketEngine& withBrownianBridge(bool b = true);
        MakeMCEuropeanBasketEngine& withAntitheticVariate(bool b = true);
        MakeMCEuropeanBasketEngine& withSamples(Size samples);
        MakeMCEuropeanBasketEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCEuropeanBasketEngine& withMaxSamples(Size samples);
        MakeMCEuropeanBasketEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator std::shared_ptr<PricingEngine>() const;
      private:
        std::shared_ptr<StochasticProcessArray> process_;
        bool brownianBridge_ = false, antithetic_ = false;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    class EuropeanMultiPathPricer : public PathPricer<MultiPath> {
      public:
        EuropeanMultiPathPricer(std::shared_ptr<BasketPayoff> payoff, DiscountFactor discount);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        std::shared_ptr<BasketPayoff> payoff_;
        DiscountFactor discount_;
    };


    // template definitions

    template <class RNG, class S>
    inline MCEuropeanBasketEngine<RNG, S>::MCEuropeanBasketEngine(
        std::shared_ptr<StochasticProcessArray> processes,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<MultiVariate, RNG, S>(antitheticVariate, false),
      processes_(std::move(processes)), timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), brownianBridge_(brownianBridge), seed_(seed) {
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
        registerWith(processes_);
    }

    template <class RNG, class S>
    inline TimeGrid MCEuropeanBasketEngine<RNG,S>::timeGrid() const {

        Time residualTime = processes_->time(
                                       this->arguments_.exercise->lastDate());
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
    std::shared_ptr<typename MCEuropeanBasketEngine<RNG,S>::path_pricer_type>
    MCEuropeanBasketEngine<RNG,S>::pathPricer() const {

        std::shared_ptr<BasketPayoff> payoff =
            std::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-basket payoff given");

        std::shared_ptr<GeneralizedBlackScholesProcess> process =
            std::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                      processes_->process(0));
        QL_REQUIRE(process, "Black-Scholes process required");

        return std::shared_ptr<
                    typename MCEuropeanBasketEngine<RNG,S>::path_pricer_type>(
            new EuropeanMultiPathPricer(payoff,
                                        process->riskFreeRate()->discount(
                                           arguments_.exercise->lastDate())));
    }


    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG, S>::MakeMCEuropeanBasketEngine(
        std::shared_ptr<StochasticProcessArray> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanBasketEngine<RNG,S>&
    MakeMCEuropeanBasketEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCEuropeanBasketEngine<RNG,S>::operator
    std::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return std::shared_ptr<PricingEngine>(new
            MCEuropeanBasketEngine<RNG,S>(process_,
                                          steps_,
                                          stepsPerYear_,
                                          brownianBridge_,
                                          antithetic_,
                                          samples_, tolerance_,
                                          maxSamples_,
                                          seed_));
    }

}


#endif

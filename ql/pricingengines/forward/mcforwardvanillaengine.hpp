/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file mcforwardvanillaengine.hpp
    \brief Monte Carlo engine for forward-starting strike-reset vanilla options
*/

#ifndef quantlib_mcforwardvanilla_engine_hpp
#define quantlib_mcforwardvanilla_engine_hpp

#include <ql/instruments/forwardvanillaoption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <utility>

namespace QuantLib {

    //! Monte Carlo engine for forward-starting vanilla options
    /*! \ingroup forwardengines
    */
    template<template <class> class MC,
             class RNG = PseudoRandom, class S = Statistics>
    class MCForwardVanillaEngine : public GenericEngine<ForwardOptionArguments<VanillaOption::arguments>,
                                                        VanillaOption::results>,
                                   public McSimulation<MC,RNG,S>
    {
      public:
        typedef typename McSimulation<MC,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MC,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MC,RNG,S>::stats_type
            stats_type;
        // constructor
        MCForwardVanillaEngine(std::shared_ptr<StochasticProcess> process,
                               Size timeSteps,
                               Size timeStepsPerYear,
                               bool brownianBridge,
                               bool antitheticVariate,
                               Size requiredSamples,
                               Real requiredTolerance,
                               Size maxSamples,
                               BigNatural seed,
                               bool controlVariate = false);
        void calculate() const override {
            McSimulation<MC,RNG,S>::calculate(requiredTolerance_,
                                              requiredSamples_,
                                              maxSamples_);
            this->results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            this->results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }

      protected:
        // McSimulation implementation
        TimeGrid timeGrid() const override;
        Real controlVariateValue() const override;
        std::shared_ptr<path_generator_type> pathGenerator() const override {

            Size dimensions = process_->factors();
            TimeGrid grid = this->timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(dimensions*(grid.size()-1),seed_);
            return std::shared_ptr<path_generator_type>(
                         new path_generator_type(process_, grid,
                                                 gen, brownianBridge_));
        }
        // data members
        std::shared_ptr<StochasticProcess> process_;
        Size timeSteps_, timeStepsPerYear_, requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    template <template <class> class MC, class RNG, class S>
    inline MCForwardVanillaEngine<MC, RNG, S>::MCForwardVanillaEngine(
        std::shared_ptr<StochasticProcess> process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed,
        bool controlVariate)
    : McSimulation<MC, RNG, S>(antitheticVariate, controlVariate), process_(std::move(process)),
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

    template <template <class> class MC, class RNG, class S>
    inline TimeGrid MCForwardVanillaEngine<MC,RNG,S>::timeGrid() const {

        Date resetDate = arguments_.resetDate;
        Date lastExerciseDate = arguments_.exercise->lastDate();

        Time t1 = process_->time(resetDate);
        Time t2 = process_->time(lastExerciseDate);

        Size totalSteps = Null<Size>();
        if (this->timeSteps_ != Null<Size>()) {
            totalSteps = timeSteps_;
        } else if (this->timeStepsPerYear_ != Null<Size>()) {
            totalSteps = static_cast<Size>(this->timeStepsPerYear_*t2);
        }

        std::vector<Time> fixingTimes;
        fixingTimes.push_back(t1);
        fixingTimes.push_back(t2);

        return TimeGrid(fixingTimes.begin(), fixingTimes.end(), totalSteps);
    }

    template <template <class> class MC, class RNG, class S>
    inline Real MCForwardVanillaEngine<MC,RNG,S>::controlVariateValue() const {

        std::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
        QL_REQUIRE(controlPE, "engine does not provide "
                              "control variation pricing engine");

        // Create vanilla option arguments with the same payoff and expiry, but with
        // strike-reset equal to initial spot*moneyness, price analytically
        std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Real spot = this->process_->initialValues()[0];
        Real moneyness = this->arguments_.moneyness;
        Real strike = moneyness * spot;

        std::shared_ptr<StrikedTypePayoff> newPayoff(new
            PlainVanillaPayoff(payoff->optionType(), strike));

        auto* controlArguments = dynamic_cast<VanillaOption::arguments*>(controlPE->getArguments());

        controlArguments->payoff = newPayoff;
        controlArguments->exercise = this->arguments_.exercise;
        controlPE->calculate();

        const auto* controlResults =
            dynamic_cast<const VanillaOption::results*>(controlPE->getResults());

        return controlResults->value;
    }
}


#endif
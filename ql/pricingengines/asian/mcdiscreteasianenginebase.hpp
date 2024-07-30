/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

/*! \file mcdiscreteasianenginebase.hpp
    \brief Monte Carlo pricing engine for discrete average Asians
*/

#ifndef quantlib_mcdiscreteasian_engine_base_hpp
#define quantlib_mcdiscreteasian_engine_base_hpp

#include <ql/exercise.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <utility>

namespace QuantLib {


    namespace detail {

        class PastFixingsOnly : public Error {
          public:
            PastFixingsOnly()
            : Error("n/a", 0, "n/a",
                    "all fixings are in the past") {}
        };

    }

    //! Pricing engine for discrete average Asians using Monte Carlo simulation
    /*! \warning control-variate calculation is disabled under VC++6.
        \ingroup asianengines
    */

    template<template <class> class MC,
             class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteAveragingAsianEngineBase :
                                public DiscreteAveragingAsianOption::engine,
                                public McSimulation<MC,RNG,S> {
      public:
        typedef
        typename McSimulation<MC,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MC,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MC,RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteAveragingAsianEngineBase(ext::shared_ptr<StochasticProcess> process,
                                           bool brownianBridge,
                                           bool antitheticVariate,
                                           bool controlVariate,
                                           Size requiredSamples,
                                           Real requiredTolerance,
                                           Size maxSamples,
                                           BigNatural seed,
                                           Size timeSteps = Null<Size>(),
                                           Size timeStepsPerYear = Null<Size>());
        void calculate() const override {
            try {
                McSimulation<MC,RNG,S>::calculate(requiredTolerance_,
                                                  requiredSamples_,
                                                  maxSamples_);
            } catch (detail::PastFixingsOnly&) {
                // Ideally, here we could calculate the payoff (which
                // is fully determine) and write it into the results.
                // This would probably need a new virtual method that
                // derived engines should implement.
                throw;
            }

            results_.value = this->mcModel_->sampleAccumulator().mean();

            if (this->controlVariate_) {
                // control variate might lead to small negative
                // option values for deep OTM options
                this->results_.value = std::max(0.0, this->results_.value);
            }

            if constexpr (RNG::allowsErrorEstimate)
                results_.errorEstimate =
                    this->mcModel_->sampleAccumulator().errorEstimate();

            // Allow inspection of the timeGrid via additional results
            this->results_.additionalResults["TimeGrid"] = this->timeGrid();
        }

      protected:
        // McSimulation implementation
        TimeGrid timeGrid() const override;
        ext::shared_ptr<path_generator_type> pathGenerator() const override {

            Size dimensions = process_->factors();
            TimeGrid grid = this->timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(dimensions*(grid.size()-1),seed_);
            return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(process_, grid,
                                                 gen, brownianBridge_));
        }
        Real controlVariateValue() const override;
        // data members
        ext::shared_ptr<StochasticProcess> process_;
        Size requiredSamples_, maxSamples_, timeSteps_, timeStepsPerYear_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    // template definitions

    template <template <class> class MC, class RNG, class S>
    inline MCDiscreteAveragingAsianEngineBase<MC, RNG, S>::MCDiscreteAveragingAsianEngineBase(
        ext::shared_ptr<StochasticProcess> process,
        bool brownianBridge,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed,
        Size timeSteps,
        Size timeStepsPerYear)
    : McSimulation<MC, RNG, S>(antitheticVariate, controlVariate), process_(std::move(process)),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples), timeSteps_(timeSteps),
      timeStepsPerYear_(timeStepsPerYear), requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {
        registerWith(process_);
    }

    template <template <class> class MC, class RNG, class S>
    inline TimeGrid MCDiscreteAveragingAsianEngineBase<MC,RNG,S>::timeGrid() const {

        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            Time t = process_->time(arguments_.fixingDates[i]);
            if (t>=0) {
                fixingTimes.push_back(t);
            }
        }

        if (fixingTimes.empty() ||
            (fixingTimes.size() == 1 && fixingTimes.front() == 0.0))
            throw detail::PastFixingsOnly();

        // Some models (eg. Heston) might request additional points in
        // the time grid to improve the accuracy of the discretization
        Date lastExerciseDate = this->arguments_.exercise->lastDate();
        Time t = process_->time(lastExerciseDate);

        if (this->timeSteps_ != Null<Size>()) {
            return TimeGrid(fixingTimes.begin(), fixingTimes.end(), timeSteps_);
        } else if (this->timeStepsPerYear_ != Null<Size>()) {
            return TimeGrid(fixingTimes.begin(), fixingTimes.end(),
                static_cast<Size>(this->timeStepsPerYear_*t));
        }

        return TimeGrid(fixingTimes.begin(), fixingTimes.end());
    }

    template<template <class> class MC, class RNG, class S>
    inline
    Real MCDiscreteAveragingAsianEngineBase<MC,RNG,S>::controlVariateValue() const {

        ext::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");

            auto* controlArguments =
                dynamic_cast<DiscreteAveragingAsianOption::arguments*>(controlPE->getArguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const auto* controlResults =
                dynamic_cast<const DiscreteAveragingAsianOption::results*>(controlPE->getResults());

            return controlResults->value;
    }

}


#endif

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

/*! \file mclookbackengine.hpp
    \brief Monte Carlo lookback fixed engines
*/

#ifndef quantlib_mc_lookback_engines_hpp
#define quantlib_mc_lookback_engines_hpp

#include <ql/exercise.hpp>
#include <ql/instruments/lookbackoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    //! Monte Carlo lookback-option engine
    template <class I, class RNG = PseudoRandom, class S = Statistics>
    class MCLookbackEngine : public I::engine,
                             public McSimulation<SingleVariate,RNG,S> {
      public:
        typedef typename McSimulation<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        // constructor
        MCLookbackEngine(std::shared_ptr<GeneralizedBlackScholesProcess> process,
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
            QL_REQUIRE(spot > 0.0, "negative or null underlying given");
            McSimulation<SingleVariate,RNG,S>::calculate(requiredTolerance_,
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
        std::shared_ptr<path_generator_type> pathGenerator() const override {
            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return std::shared_ptr<path_generator_type>(
                         new path_generator_type(process_,
                                                 grid, gen, brownianBridge_));
        }
        std::shared_ptr<path_pricer_type> pathPricer() const override;
        // data members
        std::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool antithetic_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    //! Monte Carlo lookback-option engine factory
    template <class I, class RNG = PseudoRandom, class S = Statistics>
    class MakeMCLookbackEngine {
      public:
        explicit MakeMCLookbackEngine(std::shared_ptr<GeneralizedBlackScholesProcess>);
        // named parameters
        MakeMCLookbackEngine& withSteps(Size steps);
        MakeMCLookbackEngine& withStepsPerYear(Size steps);
        MakeMCLookbackEngine& withBrownianBridge(bool b = true);
        MakeMCLookbackEngine& withAntitheticVariate(bool b = true);
        MakeMCLookbackEngine& withSamples(Size samples);
        MakeMCLookbackEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCLookbackEngine& withMaxSamples(Size samples);
        MakeMCLookbackEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator std::shared_ptr<PricingEngine>() const;
      private:
        std::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool brownianBridge_ = false, antithetic_ = false;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    // template definitions

    template <class I, class RNG, class S>
    inline MCLookbackEngine<I, RNG, S>::MCLookbackEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process,
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
        this->registerWith(process_);
    }


    template <class I, class RNG, class S>
    inline TimeGrid MCLookbackEngine<I,RNG,S>::timeGrid() const {

        Time residualTime = process_->time(this->arguments_.exercise->lastDate());
        if (timeSteps_ != Null<Size>()) {
            return TimeGrid(residualTime, timeSteps_);
        } else if (timeStepsPerYear_ != Null<Size>()) {
            Size steps = static_cast<Size>(timeStepsPerYear_*residualTime);
            return TimeGrid(residualTime, std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }


    namespace detail {

        // these functions are specialized for each of the instruments.

        std::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousFixedLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount);

        std::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousPartialFixedLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount);

        std::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousFloatingLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount);

        std::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousPartialFloatingLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount);

    }


    template <class I, class RNG, class S>
    inline std::shared_ptr<typename MCLookbackEngine<I,RNG,S>::path_pricer_type>
    MCLookbackEngine<I,RNG,S>::pathPricer() const {
        TimeGrid grid = this->timeGrid();
        DiscountFactor discount = this->process_->riskFreeRate()->discount(grid.back());

        return detail::mc_lookback_path_pricer(this->arguments_,
                                               *(this->process_),
                                               discount);
    }


    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I, RNG, S>::MakeMCLookbackEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>&
    MakeMCLookbackEngine<I,RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class I, class RNG, class S>
    inline MakeMCLookbackEngine<I,RNG,S>::operator std::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");

        return std::shared_ptr<PricingEngine>(
            new MCLookbackEngine<I,RNG,S>(process_,
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

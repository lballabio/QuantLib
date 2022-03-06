/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file mceverestengine.hpp
    \brief Monte Carlo engine for Everest options
*/

#ifndef quantlib_mc_everest_engine_hpp
#define quantlib_mc_everest_engine_hpp

#include <ql/exercise.hpp>
#include <ql/experimental/exoticoptions/everestoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <utility>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCEverestEngine : public EverestOption::engine,
                            public McSimulation<MultiVariate,RNG,S> {
      public:
        typedef typename McSimulation<MultiVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::stats_type
            stats_type;
        MCEverestEngine(ext::shared_ptr<StochasticProcessArray>,
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

            if (RNG::allowsErrorEstimate) {
                results_.errorEstimate =
                    this->mcModel_->sampleAccumulator().errorEstimate();
            }

            Real notional = arguments_.notional;
            DiscountFactor discount = endDiscount();
            results_.yield = results_.value/(notional * discount) - 1.0;
        }

      private:
        DiscountFactor endDiscount() const;
        // McEverest implementation
        TimeGrid timeGrid() const override;
        ext::shared_ptr<path_generator_type> pathGenerator() const override {

            Size numAssets = processes_->size();

            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(numAssets*(grid.size()-1),seed_);

            return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(processes_,
                                                 grid, gen, brownianBridge_));
        }
        ext::shared_ptr<path_pricer_type> pathPricer() const override;

        // data members
        ext::shared_ptr<StochasticProcessArray> processes_;
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_;
        Size maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    //! Monte Carlo Everest-option engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEverestEngine {
      public:
        explicit MakeMCEverestEngine(ext::shared_ptr<StochasticProcessArray>);
        // named parameters
        MakeMCEverestEngine& withSteps(Size steps);
        MakeMCEverestEngine& withStepsPerYear(Size steps);
        MakeMCEverestEngine& withBrownianBridge(bool b = true);
        MakeMCEverestEngine& withAntitheticVariate(bool b = true);
        MakeMCEverestEngine& withSamples(Size samples);
        MakeMCEverestEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCEverestEngine& withMaxSamples(Size samples);
        MakeMCEverestEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<StochasticProcessArray> process_;
        bool brownianBridge_, antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };


    class EverestMultiPathPricer : public PathPricer<MultiPath> {
      public:
        explicit EverestMultiPathPricer(Real notional,
                                        Rate guarantee,
                                        DiscountFactor discount);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        Real notional_;
        Rate guarantee_;
        DiscountFactor discount_;
    };


    // template definitions

    template <class RNG, class S>
    inline MCEverestEngine<RNG, S>::MCEverestEngine(
        ext::shared_ptr<StochasticProcessArray> processes,
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
    inline TimeGrid MCEverestEngine<RNG,S>::timeGrid() const {
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
    inline DiscountFactor MCEverestEngine<RNG,S>::endDiscount() const {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                      processes_->process(0));
        QL_REQUIRE(process, "Black-Scholes process required");

        return process->riskFreeRate()->discount(
                                             arguments_.exercise->lastDate());
    }

    template <class RNG, class S>
    inline ext::shared_ptr<typename MCEverestEngine<RNG,S>::path_pricer_type>
    MCEverestEngine<RNG,S>::pathPricer() const {

        return ext::shared_ptr<
                         typename MCEverestEngine<RNG,S>::path_pricer_type>(
                              new EverestMultiPathPricer(arguments_.notional,
                                                         arguments_.guarantee,
                                                         endDiscount()));
    }


    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG, S>::MakeMCEverestEngine(
        ext::shared_ptr<StochasticProcessArray> process)
    : process_(std::move(process)), brownianBridge_(false), antithetic_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()), samples_(Null<Size>()),
      maxSamples_(Null<Size>()), tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEverestEngine<RNG,S>&
    MakeMCEverestEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCEverestEngine<RNG,S>::operator
    ext::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(new
            MCEverestEngine<RNG,S>(process_,
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


#ifndef id_67106efca210ce0539e02b1ecc392bbb
#define id_67106efca210ce0539e02b1ecc392bbb
inline bool test_67106efca210ce0539e02b1ecc392bbb(const int* i) {
    return i != nullptr;
}
#endif

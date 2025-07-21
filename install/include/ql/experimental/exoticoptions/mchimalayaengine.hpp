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

/*! \file mchimalayaengine.hpp
    \brief Monte Carlo engine for Himalaya options
*/

#ifndef quantlib_mc_himalaya_engine_hpp
#define quantlib_mc_himalaya_engine_hpp

#include <ql/exercise.hpp>
#include <ql/experimental/exoticoptions/himalayaoption.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <utility>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCHimalayaEngine : public HimalayaOption::engine,
                             public McSimulation<MultiVariate,RNG,S> {
      public:
        typedef typename McSimulation<MultiVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::stats_type
            stats_type;
        MCHimalayaEngine(ext::shared_ptr<StochasticProcessArray>,
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

            if constexpr (RNG::allowsErrorEstimate)
                results_.errorEstimate =
                    this->mcModel_->sampleAccumulator().errorEstimate();
        }

      private:
        // McSimulation implementation
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
        Size requiredSamples_;
        Size maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    //! Monte Carlo Himalaya-option engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCHimalayaEngine {
      public:
        explicit MakeMCHimalayaEngine(ext::shared_ptr<StochasticProcessArray>);
        // named parameters
        MakeMCHimalayaEngine& withBrownianBridge(bool b = true);
        MakeMCHimalayaEngine& withAntitheticVariate(bool b = true);
        MakeMCHimalayaEngine& withSamples(Size samples);
        MakeMCHimalayaEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCHimalayaEngine& withMaxSamples(Size samples);
        MakeMCHimalayaEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<StochasticProcessArray> process_;
        bool brownianBridge_ = false, antithetic_ = false;
        Size samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    class HimalayaMultiPathPricer : public PathPricer<MultiPath> {
      public:
        HimalayaMultiPathPricer(ext::shared_ptr<Payoff> payoff, DiscountFactor discount);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        ext::shared_ptr<Payoff> payoff_;
        DiscountFactor discount_;
    };

    // template definitions

    template <class RNG, class S>
    inline MCHimalayaEngine<RNG, S>::MCHimalayaEngine(
        ext::shared_ptr<StochasticProcessArray> processes,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<MultiVariate, RNG, S>(antitheticVariate, false),
      processes_(std::move(processes)), requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), brownianBridge_(brownianBridge), seed_(seed) {
        registerWith(processes_);
    }

    template <class RNG, class S>
    inline TimeGrid MCHimalayaEngine<RNG,S>::timeGrid() const {

        std::vector<Time> fixingTimes;
        for (Size i=0; i<arguments_.fixingDates.size(); i++) {
            Time t = processes_->time(arguments_.fixingDates[i]);
            QL_REQUIRE(t >= 0.0, "seasoned options are not handled");
            if (i > 0) {
                QL_REQUIRE(t > fixingTimes.back(), "fixing dates not sorted");
            }
            fixingTimes.push_back(t);
        }

        return TimeGrid(fixingTimes.begin(), fixingTimes.end());
    }

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCHimalayaEngine<RNG,S>::path_pricer_type>
    MCHimalayaEngine<RNG,S>::pathPricer() const {

        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                      processes_->process(0));
        QL_REQUIRE(process, "Black-Scholes process required");

        return ext::shared_ptr<
                         typename MCHimalayaEngine<RNG,S>::path_pricer_type>(
            new HimalayaMultiPathPricer(arguments_.payoff,
                                        process->riskFreeRate()->discount(
                                           arguments_.exercise->lastDate())));
    }


    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG, S>::MakeMCHimalayaEngine(
        ext::shared_ptr<StochasticProcessArray> process)
    : process_(std::move(process)), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHimalayaEngine<RNG,S>&
    MakeMCHimalayaEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCHimalayaEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        return ext::shared_ptr<PricingEngine>(new
            MCHimalayaEngine<RNG,S>(process_,
                                    brownianBridge_,
                                    antithetic_,
                                    samples_,
                                    tolerance_,
                                    maxSamples_,
                                    seed_));
    }

}

#endif

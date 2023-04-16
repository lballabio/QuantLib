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

/*! \file mcforwardeuropeanbsengine.hpp
    \brief Monte Carlo engine for forward-starting strike-reset European options using BS process
*/

#ifndef quantlib_mc_forward_european_bs_engine_hpp
#define quantlib_mc_forward_european_bs_engine_hpp

#include <ql/pricingengines/forward/mcforwardvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    /*! \ingroup forwardengines
        \test
        - the correctness of the returned value is tested by
          comparing prices to the analytic pricer for a range
          of moneynesses
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCForwardEuropeanBSEngine
        : public MCForwardVanillaEngine<SingleVariate,RNG,S> {
      public:
        typedef
        typename MCForwardVanillaEngine<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename MCForwardVanillaEngine<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCForwardVanillaEngine<SingleVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCForwardEuropeanBSEngine(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
      protected:
        std::shared_ptr<path_pricer_type> pathPricer() const override;
    };


    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCForwardEuropeanBSEngine {
      public:
        explicit MakeMCForwardEuropeanBSEngine(
            std::shared_ptr<GeneralizedBlackScholesProcess> process);
        // named parameters
        MakeMCForwardEuropeanBSEngine& withSteps(Size steps);
        MakeMCForwardEuropeanBSEngine& withStepsPerYear(Size steps);
        MakeMCForwardEuropeanBSEngine& withBrownianBridge(bool b = false);
        MakeMCForwardEuropeanBSEngine& withSamples(Size samples);
        MakeMCForwardEuropeanBSEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCForwardEuropeanBSEngine& withMaxSamples(Size samples);
        MakeMCForwardEuropeanBSEngine& withSeed(BigNatural seed);
        MakeMCForwardEuropeanBSEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator std::shared_ptr<PricingEngine>() const;
      private:
        std::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_ = false;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_ = false;
        BigNatural seed_ = 0;
    };


    class ForwardEuropeanBSPathPricer : public PathPricer<Path> {
      public:
        ForwardEuropeanBSPathPricer(Option::Type type,
                                   Real moneyness,
                                   Size resetIndex,
                                   DiscountFactor discount);
        Real operator()(const Path& path) const override;

      private:
        Option::Type type_;
        Real moneyness_;
        Size resetIndex_;
        DiscountFactor discount_;
    };


    // inline definitions

    template <class RNG, class S>
    inline
    MCForwardEuropeanBSEngine<RNG,S>::MCForwardEuropeanBSEngine(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : MCForwardVanillaEngine<SingleVariate,RNG,S>(process,
                                                  timeSteps,
                                                  timeStepsPerYear,
                                                  brownianBridge,
                                                  antitheticVariate,
                                                  requiredSamples,
                                                  requiredTolerance,
                                                  maxSamples,
                                                  seed) {}


    template <class RNG, class S>
    inline
    std::shared_ptr<typename MCForwardEuropeanBSEngine<RNG,S>::path_pricer_type>
        MCForwardEuropeanBSEngine<RNG,S>::pathPricer() const {

        TimeGrid timeGrid = this->timeGrid();

        Time resetTime = this->process_->time(this->arguments_.resetDate);
        Size resetIndex = timeGrid.closestIndex(resetTime);

        std::shared_ptr<PlainVanillaPayoff> payoff =
            std::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        std::shared_ptr<EuropeanExercise> exercise =
            std::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        std::shared_ptr<GeneralizedBlackScholesProcess> process =
            std::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                this->process_);
        QL_REQUIRE(process, "Black-Scholes process required");

        return std::shared_ptr<typename
            MCForwardEuropeanBSEngine<RNG,S>::path_pricer_type>(
                new ForwardEuropeanBSPathPricer(
                                        payoff->optionType(),
                                        this->arguments_.moneyness,
                                        resetIndex,
                                        process->riskFreeRate()->discount(
                                                   timeGrid.back())));
    }


    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG, S>::MakeMCForwardEuropeanBSEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withAbsoluteTolerance(
                                                             Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withBrownianBridge(bool b) {
        brownianBridge_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCForwardEuropeanBSEngine<RNG,S>&
    MakeMCForwardEuropeanBSEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCForwardEuropeanBSEngine<RNG,S>::operator std::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified - set EITHER steps OR stepsPerYear");
        return std::shared_ptr<PricingEngine>(new
            MCForwardEuropeanBSEngine<RNG,S>(process_,
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

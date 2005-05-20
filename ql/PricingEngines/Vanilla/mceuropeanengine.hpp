/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mceuropeanengine.hpp
    \brief Monte Carlo European option engine
*/

#ifndef quantlib_montecarlo_european_engine_hpp
#define quantlib_montecarlo_european_engine_hpp

#include <ql/PricingEngines/Vanilla/mcvanillaengine.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Volatilities/blackvariancecurve.hpp>

namespace QuantLib {

    //! European option pricing engine using Monte Carlo simulation
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              checking it against analytic results.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCEuropeanEngine : public MCVanillaEngine<RNG,S> {
      public:
        typedef typename MCVanillaEngine<RNG,S>::path_generator_type
            path_generator_type;
        typedef typename MCVanillaEngine<RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCVanillaEngine<RNG,S>::stats_type
            stats_type;
        // constructor
        MCEuropeanEngine(Size timeSteps,
                         Size timeStepsPerYear,
                         bool brownianBridge,
                         bool antitheticVariate,
                         bool controlVariate,
                         Size requiredSamples,
                         Real requiredTolerance,
                         Size maxSamples,
                         BigNatural seed);
      protected:
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };

    //! Monte Carlo European engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEuropeanEngine {
      public:
        MakeMCEuropeanEngine();
        // named parameters
        MakeMCEuropeanEngine& withSteps(Size steps);
        MakeMCEuropeanEngine& withStepsPerYear(Size steps);
        MakeMCEuropeanEngine& withBrownianBridge(bool b = true);
        MakeMCEuropeanEngine& withSamples(Size samples);
        MakeMCEuropeanEngine& withTolerance(Real tolerance);
        MakeMCEuropeanEngine& withMaxSamples(Size samples);
        MakeMCEuropeanEngine& withSeed(BigNatural seed);
        MakeMCEuropeanEngine& withAntitheticVariate(bool b = true);
        MakeMCEuropeanEngine& withControlVariate(bool b = true);
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_, controlVariate_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    class EuropeanPathPricer : public PathPricer<Path> {
      public:
        EuropeanPathPricer(Option::Type type,
                           Real underlying,
                           Real strike,
                           DiscountFactor discount);
        Real operator()(const Path& path) const;
      private:
        Real underlying_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    // inline definitions

    template <class RNG, class S>
    inline
    MCEuropeanEngine<RNG,S>::MCEuropeanEngine(Size timeSteps,
                                              Size timeStepsPerYear,
                                              bool brownianBridge,
                                              bool antitheticVariate,
                                              bool controlVariate,
                                              Size requiredSamples,
                                              Real requiredTolerance,
                                              Size maxSamples,
                                              BigNatural seed)
    : MCVanillaEngine<RNG,S>(timeSteps,
                             timeStepsPerYear,
                             brownianBridge,
                             antitheticVariate,
                             controlVariate,
                             requiredSamples,
                             requiredTolerance,
                             maxSamples,
                             seed) {}


    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCEuropeanEngine<RNG,S>::path_pricer_type>
    MCEuropeanEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                this->arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        return boost::shared_ptr<
                       QL_TYPENAME MCEuropeanEngine<RNG,S>::path_pricer_type>(
          new EuropeanPathPricer(
              payoff->optionType(),
              process->stateVariable()->value(),
              payoff->strike(),
              process->riskFreeRate()->discount(this->timeGrid().back())));
    }


    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>::MakeMCEuropeanEngine()
    : antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), brownianBridge_(false), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCEuropeanEngine<RNG,S>::operator boost::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return boost::shared_ptr<PricingEngine>(new
            MCEuropeanEngine<RNG,S>(steps_,
                                    stepsPerYear_,
                                    brownianBridge_,
                                    antithetic_,
                                    controlVariate_,
                                    samples_, tolerance_,
                                    maxSamples_,
                                    seed_));
    }



    inline EuropeanPathPricer::EuropeanPathPricer(
                            Option::Type type,
                            Real underlying, Real strike,
                            DiscountFactor discount)
    : underlying_(underlying), payoff_(type, strike), discount_(discount) {
        QL_REQUIRE(underlying>0.0,
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

    inline Real EuropeanPathPricer::operator()(const Path& path) const {
        #ifndef QL_DISABLE_DEPRECATED
        Size n = path.size();
        #else
        Size n = path.length();
        #endif
        QL_REQUIRE(n>0, "the path cannot be empty");

        #ifndef QL_DISABLE_DEPRECATED
        Real log_variation = 0.0;
        for (Size i = 0; i < n; i++)
            log_variation += path[i];

        return payoff_(underlying_ * std::exp(log_variation)) * discount_;
        #else
        return payoff_(path.back()) * discount_;
        #endif
    }

}


#endif

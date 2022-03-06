/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file mcdigitalengine.hpp
    \brief digital option Monte Carlo engine
*/

#ifndef quantlib_digital_mc_engine_hpp
#define quantlib_digital_mc_engine_hpp

#include <ql/exercise.hpp>
#include <ql/methods/montecarlo/mctraits.hpp>
#include <ql/pricingengines/vanilla/mcvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Pricing engine for digital options using Monte Carlo simulation
    /*! Uses the Brownian Bridge correction for the barrier found in
        <i>
        Going to Extremes: Correcting Simulation Bias in Exotic
        Option Valuation - D.R. Beaglehole, P.H. Dybvig and G. Zhou
        Financial Analysts Journal; Jan/Feb 1997; 53, 1. pg. 62-68
        </i>
        and
        <i>
        Simulating path-dependent options: A new approach -
        M. El Babsiri and G. Noel
        Journal of Derivatives; Winter 1998; 6, 2; pg. 65-83
        </i>

        \ingroup vanillaengines

        \test the correctness of the returned value in case of
              cash-or-nothing at-hit digital payoff is tested by
              reproducing known good results.
    */
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCDigitalEngine : public MCVanillaEngine<SingleVariate,RNG,S> {
      public:
        typedef
        typename MCVanillaEngine<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename MCVanillaEngine<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCVanillaEngine<SingleVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCDigitalEngine(
                    const ext::shared_ptr<GeneralizedBlackScholesProcess>&,
                    Size timeSteps,
                    Size timeStepsPerYear,
                    bool brownianBridge,
                    bool antitheticVariate,
                    Size requiredSamples,
                    Real requiredTolerance,
                    Size maxSamples,
                    BigNatural seed);
      protected:
        // McSimulation implementation
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
    };

    //! Monte Carlo digital engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDigitalEngine {
      public:
        MakeMCDigitalEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        // named parameters
        MakeMCDigitalEngine& withSteps(Size steps);
        MakeMCDigitalEngine& withStepsPerYear(Size steps);
        MakeMCDigitalEngine& withBrownianBridge(bool b = true);
        MakeMCDigitalEngine& withSamples(Size samples);
        MakeMCDigitalEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDigitalEngine& withMaxSamples(Size samples);
        MakeMCDigitalEngine& withSeed(BigNatural seed);
        MakeMCDigitalEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    class DigitalPathPricer : public PathPricer<Path> {
      public:
        DigitalPathPricer(ext::shared_ptr<CashOrNothingPayoff> payoff,
                          ext::shared_ptr<AmericanExercise> exercise,
                          Handle<YieldTermStructure> discountTS,
                          ext::shared_ptr<StochasticProcess1D> diffProcess,
                          PseudoRandom::ursg_type sequenceGen);
        Real operator()(const Path& path) const override;

      private:
        ext::shared_ptr<CashOrNothingPayoff> payoff_;
        ext::shared_ptr<AmericanExercise> exercise_;
        ext::shared_ptr<StochasticProcess1D> diffProcess_;
        PseudoRandom::ursg_type sequenceGen_;
        Handle<YieldTermStructure> discountTS_;
    };



    // template definitions

    template<class RNG, class S>
    MCDigitalEngine<RNG,S>::MCDigitalEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : MCVanillaEngine<SingleVariate,RNG,S>(process,
                                           timeSteps,
                                           timeStepsPerYear,
                                           brownianBridge,
                                           antitheticVariate,
                                           false,
                                           requiredSamples,
                                           requiredTolerance,
                                           maxSamples,
                                           seed) {}

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCDigitalEngine<RNG,S>::path_pricer_type>
    MCDigitalEngine<RNG,S>::pathPricer() const {

        ext::shared_ptr<CashOrNothingPayoff> payoff =
            ext::dynamic_pointer_cast<CashOrNothingPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "wrong payoff given");

        ext::shared_ptr<AmericanExercise> exercise =
            ext::dynamic_pointer_cast<AmericanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "Black-Scholes process required");

        TimeGrid grid = this->timeGrid();
        PseudoRandom::ursg_type sequenceGen(grid.size()-1,
                                            PseudoRandom::urng_type(76));

        return ext::shared_ptr<
                        typename MCDigitalEngine<RNG,S>::path_pricer_type>(
          new DigitalPathPricer(payoff,
                                exercise,
                                process->riskFreeRate(),
                                process,
                                sequenceGen));
    }


    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG, S>::MakeMCDigitalEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), antithetic_(false), steps_(Null<Size>()),
      stepsPerYear_(Null<Size>()), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), brownianBridge_(false), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDigitalEngine<RNG,S>&
    MakeMCDigitalEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDigitalEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(new
            MCDigitalEngine<RNG,S>(process_,
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


#ifndef id_d06c47e1f7213a6628b64c1a74191a4c
#define id_d06c47e1f7213a6628b64c1a74191a4c
inline bool test_d06c47e1f7213a6628b64c1a74191a4c(int* i) { return i != 0; }
#endif

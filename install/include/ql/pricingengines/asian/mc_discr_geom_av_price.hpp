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

/*! \file mc_discr_geom_av_price.hpp
    \brief Monte Carlo engine for discrete geometric average price Asian
*/

#ifndef quantlib_mc_discrete_geometric_average_price_asian_engine_h
#define quantlib_mc_discrete_geometric_average_price_asian_engine_h

#include <ql/exercise.hpp>
#include <ql/pricingengines/asian/mcdiscreteasianenginebase.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <utility>

namespace QuantLib {

    //!  Monte Carlo pricing engine for discrete geometric average price Asian
    /*! \ingroup asianengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteGeometricAPEngine
        : public MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S> {
      public:
        typedef
        typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteGeometricAPEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
    };


    class GeometricAPOPathPricer : public PathPricer<Path> {
      public:
        GeometricAPOPathPricer(Option::Type type,
                               Real strike,
                               DiscountFactor discount,
                               Real runningProduct = 1.0,
                               Size pastFixings = 0);
        Real operator()(const Path& path) const override;

      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
        Real runningProduct_;
        Size pastFixings_;
    };


    // inline definitions

    template <class RNG, class S>
    inline
    MCDiscreteGeometricAPEngine<RNG,S>::MCDiscreteGeometricAPEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>(process,
                                                              brownianBridge,
                                                              antitheticVariate,
                                                              false,
                                                              requiredSamples,
                                                              requiredTolerance,
                                                              maxSamples,
                                                              seed) {}



    template <class RNG, class S>
    inline
    ext::shared_ptr<
            typename MCDiscreteGeometricAPEngine<RNG,S>::path_pricer_type>
        MCDiscreteGeometricAPEngine<RNG,S>::pathPricer() const {

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                this->process_);
        QL_REQUIRE(process, "Black-Scholes process required");

        return ext::shared_ptr<typename
            MCDiscreteGeometricAPEngine<RNG,S>::path_pricer_type>(
                new GeometricAPOPathPricer(
                    payoff->optionType(),
                    payoff->strike(),
                    process->riskFreeRate()->discount(exercise->lastDate()),
                    this->arguments_.runningAccumulator,
                    this->arguments_.pastFixings));
    }


    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteGeometricAPEngine {
      public:
        explicit MakeMCDiscreteGeometricAPEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        // named parameters
        MakeMCDiscreteGeometricAPEngine& withBrownianBridge(bool b = true);
        MakeMCDiscreteGeometricAPEngine& withSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDiscreteGeometricAPEngine& withMaxSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withSeed(BigNatural seed);
        MakeMCDiscreteGeometricAPEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_ = false;
        Size samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_ = true;
        BigNatural seed_ = 0;
    };

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG, S>::MakeMCDiscreteGeometricAPEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withAbsoluteTolerance(
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
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withBrownianBridge(bool b) {
        brownianBridge_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDiscreteGeometricAPEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        return ext::shared_ptr<PricingEngine>(new
            MCDiscreteGeometricAPEngine<RNG,S>(process_,
                                               brownianBridge_,
                                               antithetic_,
                                               samples_, tolerance_,
                                               maxSamples_,
                                               seed_));
    }

}


#endif

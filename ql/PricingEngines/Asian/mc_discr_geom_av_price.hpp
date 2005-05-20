/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

/*! \file mc_discr_geom_av_price.hpp
    \brief Monte Carlo engine for discrete geometric average price Asian
*/

#ifndef quantlib_mc_discrete_geometric_average_price_asian_engine_h
#define quantlib_mc_discrete_geometric_average_price_asian_engine_h

#include <ql/PricingEngines/Asian/mcdiscreteasianengine.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Volatilities/blackvariancecurve.hpp>

namespace QuantLib {

    //!  Monte Carlo pricing engine for discrete geometric average price Asian
    /*! \ingroup asianengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteGeometricAPEngine : public MCDiscreteAveragingAsianEngine<RNG,S> {
      public:
        typedef typename MCDiscreteAveragingAsianEngine<RNG,S>::path_generator_type
            path_generator_type;
        typedef typename MCDiscreteAveragingAsianEngine<RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngine<RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteGeometricAPEngine(Size maxTimeStepPerYear,
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


    class GeometricAPOPathPricer : public PathPricer<Path> {
      public:
        GeometricAPOPathPricer(Option::Type type,
                               Real underlying,
                               Real strike,
                               DiscountFactor discount,
                               Real runningProduct = 1.0,
                               Size pastFixings = 0);
        Real operator()(const Path& path) const {
            #ifndef QL_DISABLE_DEPRECATED
            Size n = path.size();
            #else
            Size n = path.length() - 1;
            #endif
            QL_REQUIRE(n>0, "the path cannot be empty");

            Real averagePrice;
            #ifndef QL_DISABLE_DEPRECATED
            Real runningLog = runningLog_;
            // path[i] is d log(S), the log increment
            for (Size i=0; i<n; i++)
                runningLog += (n-i)*path[i];
            // not sure the if case is correct
            if (path.timeGrid().mandatoryTimes()[0]==0.0)
                averagePrice = underlying_ *
                                    std::exp(runningLog/(n+pastFixings_+1));
            else
                averagePrice = underlying_ *
                                    std::exp(runningLog/(n+pastFixings_));
            #else
            Real product = runningProduct_;
            Size fixings = n+pastFixings_;
            if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                fixings += 1;
                product *= underlying_;
            }
            // care must be taken not to overflow product
            Real maxValue = QL_MAX_REAL;
            averagePrice = 1.0;
            for (Size i=1; i<n+1; i++) {
                Real price = path.value(i);
                if (product < maxValue/price) {
                    product *= price;
                } else {
                    averagePrice *= std::pow(product, 1.0/fixings);
                    product = price;
                }
            }
            averagePrice *= std::pow(product, 1.0/fixings);
            #endif
            return discount_ * payoff_(averagePrice);
        }
      private:
        Real underlying_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
        Real runningProduct_, runningLog_;
        Size pastFixings_;
    };


    // inline definitions

    template <class RNG, class S>
    inline
    MCDiscreteGeometricAPEngine<RNG,S>::MCDiscreteGeometricAPEngine(
                                                    Size maxTimeStepPerYear,
                                                    bool brownianBridge,
                                                    bool antitheticVariate,
                                                    bool controlVariate,
                                                    Size requiredSamples,
                                                    Real requiredTolerance,
                                                    Size maxSamples,
                                                    BigNatural seed)
    : MCDiscreteAveragingAsianEngine<RNG,S>(maxTimeStepPerYear,
                                            brownianBridge,
                                            antitheticVariate,
                                            controlVariate,
                                            requiredSamples,
                                            requiredTolerance,
                                            maxSamples,
                                            seed) {}



    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCDiscreteGeometricAPEngine<RNG,S>::path_pricer_type>
        MCDiscreteGeometricAPEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<EuropeanExercise> exercise =
            boost::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                          this->arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        return boost::shared_ptr<QL_TYPENAME
            MCDiscreteGeometricAPEngine<RNG,S>::path_pricer_type>(
            new GeometricAPOPathPricer(
              payoff->optionType(),
              process->stateVariable()->value(),
              payoff->strike(),
              process->riskFreeRate()->discount(this->timeGrid().back())));
    }


    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteGeometricAPEngine {
      public:
        MakeMCDiscreteGeometricAPEngine();
        // named parameters
        MakeMCDiscreteGeometricAPEngine& withStepsPerYear(Size maxSteps);
        MakeMCDiscreteGeometricAPEngine& withBrownianBridge(bool b = true);
        MakeMCDiscreteGeometricAPEngine& withSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withTolerance(Real tolerance);
        MakeMCDiscreteGeometricAPEngine& withMaxSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withSeed(BigNatural seed);
        MakeMCDiscreteGeometricAPEngine& withAntitheticVariate(bool b = true);
        MakeMCDiscreteGeometricAPEngine& withControlVariate(bool b = true);
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_, controlVariate_;
        Size steps_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>::MakeMCDiscreteGeometricAPEngine()
    : antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), brownianBridge_(true), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withStepsPerYear(Size maxSteps) {
        steps_ = maxSteps;
        return *this;
    }

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
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withTolerance(Real tolerance) {
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
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDiscreteGeometricAPEngine<RNG,S>::operator boost::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>(),
                   "max number of steps per year not given");
        return boost::shared_ptr<PricingEngine>(new
            MCDiscreteGeometricAPEngine<RNG,S>(steps_,
                                               brownianBridge_,
                                               antithetic_, controlVariate_,
                                               samples_, tolerance_,
                                               maxSamples_,
                                               seed_));
    }



}


#endif

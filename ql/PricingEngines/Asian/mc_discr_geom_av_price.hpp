
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

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
    /*! \ingroup asianengines */
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
                            bool antitheticVariate = false,
                            bool controlVariate = false,
                            Size requiredSamples = Null<Size>(),
                            Real requiredTolerance = Null<Real>(),
                            Size maxSamples = Null<Size>(),
                            BigNatural seed = 0) {}
      protected:
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };



    class GeometricAPOPathPricer : public PathPricer<Path> {
        public:
        GeometricAPOPathPricer(Option::Type type,
                                Real underlying,
                                Real strike,
                                DiscountFactor discount);
        Real operator()(const Path& path) const {
            Size n = path.size();
            QL_REQUIRE(n>0, "the path cannot be empty");
            Real geoLogVariation = 0.0;
            for (Size i=0; i<n; i++)
                geoLogVariation += (n-i)*path[i];
            Real averagePrice1;
            if (path.timeGrid().mandatoryTimes()[0]==0.0)
                averagePrice1=underlying_*QL_EXP(geoLogVariation/(n+1));
            else
                averagePrice1=underlying_*QL_EXP(geoLogVariation/n);
            return discount_ * payoff_(averagePrice1);
        }
        private:
        Real underlying_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    // inline definitions

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

        return boost::shared_ptr<QL_TYPENAME
            MCDiscreteGeometricAPEngine<RNG,S>::path_pricer_type>(
            new GeometricAPOPathPricer(
              payoff->optionType(),
              this->arguments_.blackScholesProcess->stateVariable()->value(),
              payoff->strike(),
              this->arguments_.blackScholesProcess->riskFreeRate()
                                        ->discount(this->timeGrid().back())));
    }


    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteGeometricAPEngine {
      public:
        MakeMCDiscreteGeometricAPEngine();
        // named parameters
        MakeMCDiscreteGeometricAPEngine& withStepsPerYear(Size maxSteps);
        MakeMCDiscreteGeometricAPEngine& withSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withTolerance(Real tolerance);
        MakeMCDiscreteGeometricAPEngine& withMaxSamples(Size samples);
        MakeMCDiscreteGeometricAPEngine& withSeed(BigNatural seed);
        MakeMCDiscreteGeometricAPEngine& withAntitheticVariate();
        MakeMCDiscreteGeometricAPEngine& withControlVariate();
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_, controlVariate_;
        Size steps_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>::MakeMCDiscreteGeometricAPEngine()
    : antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}

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
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withAntitheticVariate() {
        antithetic_ = true;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteGeometricAPEngine<RNG,S>&
    MakeMCDiscreteGeometricAPEngine<RNG,S>::withControlVariate() {
        controlVariate_ = true;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDiscreteGeometricAPEngine<RNG,S>::operator boost::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>(),
                   "max number of steps per year not given");
        return boost::shared_ptr<PricingEngine>(
                             new MCDiscreteGeometricAPEngine<RNG,S>(steps_, antithetic_,
                                                         controlVariate_,
                                                         samples_, tolerance_,
                                                         maxSamples_, seed_));
    }



}


#endif

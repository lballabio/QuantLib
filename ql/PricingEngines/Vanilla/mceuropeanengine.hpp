
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file mceuropeanengine.hpp
    \brief Monte Carlo European option engine
*/

#ifndef quantlib_montecarlo_european_engine_h
#define quantlib_montecarlo_european_engine_h

#include <ql/PricingEngines/Vanilla/mcvanillaengine.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Volatilities/blackvariancecurve.hpp>

namespace QuantLib {

    //! European option pricing engine using Monte Carlo simulation
    /*! \ingroup vanillaengines */
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
        MCEuropeanEngine(Size maxTimeStepPerYear,
                         bool antitheticVariate = false,
                         bool controlVariate = false,
                         Size requiredSamples = Null<Size>(),
                         Real requiredTolerance = Null<Real>(),
                         Size maxSamples = Null<Size>(),
                         BigNatural seed = 0);
      protected:
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };

    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEuropeanEngine {
      public:
        MakeMCEuropeanEngine();
        // named parameters
        MakeMCEuropeanEngine& withStepsPerYear(Size maxSteps);
        MakeMCEuropeanEngine& withSamples(Size samples);
        MakeMCEuropeanEngine& withTolerance(Real tolerance);
        MakeMCEuropeanEngine& withMaxSamples(Size samples);
        MakeMCEuropeanEngine& withSeed(BigNatural seed);
        MakeMCEuropeanEngine& withAntitheticVariate();
        MakeMCEuropeanEngine& withControlVariate();
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_, controlVariate_;
        Size steps_, samples_, maxSamples_;
        Real tolerance_;
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
    MCEuropeanEngine<RNG,S>::MCEuropeanEngine(Size maxTimeStepPerYear,
                                              bool antitheticVariate,
                                              bool controlVariate,
                                              Size requiredSamples,
                                              Real requiredTolerance,
                                              Size maxSamples,
                                              BigNatural seed)
    : MCVanillaEngine<RNG,S>(maxTimeStepPerYear,
                             antitheticVariate,
                             controlVariate,
                             requiredSamples,
                             requiredTolerance,
                             maxSamples,
                             seed) {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");
                             }


    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCEuropeanEngine<RNG,S>::path_pricer_type>
    MCEuropeanEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        return boost::shared_ptr<
                       QL_TYPENAME MCEuropeanEngine<RNG,S>::path_pricer_type>(
          new EuropeanPathPricer(
              payoff->optionType(),
              this->arguments_.blackScholesProcess->stateVariable()->value(),
              payoff->strike(),
              this->arguments_.blackScholesProcess->riskFreeRate()
                                        ->discount(this->timeGrid().back())));
    }


    namespace {

        // not appropriate for path-dependent and American options
        class TimeGridCalculator : public AcyclicVisitor,
                                   public Visitor<BlackVolTermStructure>,
                                   public Visitor<BlackConstantVol>,
                                   public Visitor<BlackVarianceCurve> {
          public:
            TimeGridCalculator(Time maturity, Size stepsPerYear)
            : maturity_(maturity), stepsPerYear_(stepsPerYear) {}
            Size size() { return result_; }
            // generic case
            void visit(BlackVolTermStructure&) {
                result_ = Size(QL_MAX<Real>(maturity_ * stepsPerYear_, 1.0));
            }
            // specializations
            void visit(BlackConstantVol&) {
                result_ = 1;
            }
            void visit(BlackVarianceCurve&) {
                result_ = 1;
            }
          private:
            Time maturity_;
            Size stepsPerYear_;
            Size result_;
        };

    }

    template <class RNG, class S>
    inline TimeGrid MCEuropeanEngine<RNG,S>::timeGrid() const {

        Date refDate = this->arguments_.blackScholesProcess->
            riskFreeRate()->referenceDate();

        Time t = this->arguments_.blackScholesProcess->riskFreeRate()
            ->dayCounter().yearFraction(
              refDate, this->arguments_.exercise->lastDate());

        TimeGridCalculator calc(t, this->maxTimeStepsPerYear_);
        this->arguments_.blackScholesProcess->blackVolatility()->accept(calc);
        return TimeGrid(t, calc.size());
    }



    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>::MakeMCEuropeanEngine()
    : antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withStepsPerYear(Size maxSteps) {
        steps_ = maxSteps;
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
    MakeMCEuropeanEngine<RNG,S>::withAntitheticVariate() {
        antithetic_ = true;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withControlVariate() {
        controlVariate_ = true;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCEuropeanEngine<RNG,S>::operator boost::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>(),
                   "max number of steps per year not given");
        return boost::shared_ptr<PricingEngine>(
                             new MCEuropeanEngine<RNG,S>(steps_, antithetic_,
                                                         controlVariate_,
                                                         samples_, tolerance_,
                                                         maxSamples_, seed_));
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
        Size n = path.size();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Real log_variation = 0.0;
        for (Size i = 0; i < n; i++)
            log_variation += path[i];

        return payoff_(underlying_ * QL_EXP(log_variation)) * discount_;
    }

}


#endif


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
                         Size requiredSamples = Null<int>(),
                         double requiredTolerance = Null<double>(),
                         Size maxSamples = Null<int>(),
                         long seed = 0);
      protected:
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };

    #if !defined(QL_PATCH_MICROSOFT)
    // Visual cannot cope with the conversion operator to boost::shared_ptr
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEuropeanEngine {
      public:
        MakeMCEuropeanEngine();
        // named parameters
        MakeMCEuropeanEngine& withStepsPerYear(Size maxSteps);
        MakeMCEuropeanEngine& withSamples(Size samples);
        MakeMCEuropeanEngine& withTolerance(double tolerance);
        MakeMCEuropeanEngine& withMaxSamples(Size samples);
        MakeMCEuropeanEngine& withSeed(long seed);
        MakeMCEuropeanEngine& withAntitheticVariate();
        MakeMCEuropeanEngine& withControlVariate();
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_, controlVariate_;
        Size steps_, samples_, maxSamples_;
        double tolerance_;
        long seed_;
    };
    #endif

    class EuropeanPathPricer : public PathPricer<Path> {
      public:
        EuropeanPathPricer(Option::Type type,
                           double underlying,
                           double strike,
                           const RelinkableHandle<TermStructure>& discountTS);
        double operator()(const Path& path) const;
      private:
        double underlying_;
        PlainVanillaPayoff payoff_;
    };


    // inline definitions

    template <class RNG, class S>
    inline
    MCEuropeanEngine<RNG,S>::MCEuropeanEngine(Size maxTimeStepPerYear,
                                              bool antitheticVariate,
                                              bool controlVariate,
                                              Size requiredSamples,
                                              double requiredTolerance,
                                              Size maxSamples,
                                              long seed)
    : MCVanillaEngine<RNG,S>(maxTimeStepPerYear,
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
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "AnalyticAmericanBinaryEngine: non-plain payoff given");

        return boost::shared_ptr<MCEuropeanEngine<RNG,S>::path_pricer_type>(
            new EuropeanPathPricer(
                payoff->optionType(),
                arguments_.blackScholesProcess->stateVariable->value(),
                payoff->strike(),
                arguments_.blackScholesProcess->riskFreeTS));
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
                result_ = Size(QL_MAX(maturity_ * stepsPerYear_, 1.0));
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

        Time t = arguments_.blackScholesProcess->riskFreeTS
            ->dayCounter().yearFraction(
                arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
                arguments_.exercise->lastDate());

        TimeGridCalculator calc(t, maxTimeStepsPerYear_);
        arguments_.blackScholesProcess->volTS->accept(calc);
        return TimeGrid(t, calc.size());
    }


    #if !defined(QL_PATCH_MICROSOFT)

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>::MakeMCEuropeanEngine()
    : antithetic_(false), controlVariate_(false),
      steps_(Null<int>()), samples_(Null<int>()), maxSamples_(Null<int>()),
      tolerance_(Null<double>()), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withStepsPerYear(Size maxSteps) {
        steps_ = maxSteps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<double>(),
                   "MakeMCEuropeanEngine::withSamples: "
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine<RNG,S>&
    MakeMCEuropeanEngine<RNG,S>::withTolerance(double tolerance) {
        QL_REQUIRE(samples_ == Size(Null<int>()),
                   "MakeMCEuropeanEngine::withTolerance: "
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "MakeMCEuropeanEngine::withTolerance: "
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
    MakeMCEuropeanEngine<RNG,S>::withSeed(long seed) {
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
        QL_REQUIRE(steps_ != Size(Null<int>()),
                   "MakeMCEuropeanEngine<RNG,S>: "
                   "max number of steps per year not given");
        return boost::shared_ptr<PricingEngine>(
                             new MCEuropeanEngine<RNG,S>(steps_, antithetic_,
                                                         controlVariate_,
                                                         samples_, tolerance_,
                                                         maxSamples_, seed_));
    }

    #endif


    inline EuropeanPathPricer::EuropeanPathPricer(
                            Option::Type type,
                            double underlying, double strike,
                            const RelinkableHandle<TermStructure>& discountTS)
    : PathPricer<Path>(discountTS), underlying_(underlying),
      payoff_(type, strike) {
        QL_REQUIRE(underlying>0.0,
                   "EuropeanPathPricer: "
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>=0.0,
                   "EuropeanPathPricer: "
                   "strike less than zero not allowed");
    }

    inline double EuropeanPathPricer::operator()(const Path& path) const {
        Size n = path.size();
        QL_REQUIRE(n>0,
                   "EuropeanPathPricer: the path cannot be empty");

        double log_variation = 0.0;
        for (Size i = 0; i < n; i++)
            log_variation += path[i];

        return payoff_(underlying_ * QL_EXP(log_variation)) *
                       discountTS_->discount(path.timeGrid().back());
    }

}


#endif

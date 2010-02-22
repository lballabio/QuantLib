/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
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

/*! \file mcamericanengine.hpp
    \brief American Monte Carlo engine
*/

#ifndef quantlib_mc_american_engine_hpp
#define quantlib_mc_american_engine_hpp

#include <ql/qldefines.hpp>
#include <ql/payoff.hpp>
#include <ql/exercise.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/mclongstaffschwartzengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>

namespace QuantLib {

    //! American Monte Carlo engine
    /*! References:

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCAmericanEngine
        : public MCLongstaffSchwartzEngine<VanillaOption::engine,
                                           SingleVariate,RNG,S>{
      public:
        MCAmericanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool antitheticVariate,
             bool controlVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed,
             Size polynomOrder,
             LsmBasisSystem::PolynomType polynomType,
             Size nCalibrationSamples = Null<Size>());

        void calculate() const;
        
      protected:
        boost::shared_ptr<LongstaffSchwartzPathPricer<Path> >
            lsmPathPricer() const;

        Real controlVariateValue() const;
        boost::shared_ptr<PricingEngine> controlPricingEngine() const;
        boost::shared_ptr<PathPricer<Path> > controlPathPricer() const;

      private:
        const Size polynomOrder_;
        const LsmBasisSystem::PolynomType polynomType_;
    };

    class AmericanPathPricer : public EarlyExercisePathPricer<Path>  {
      public:
        AmericanPathPricer(const boost::shared_ptr<Payoff>& payoff,
                           Size polynomOrder,
                           LsmBasisSystem::PolynomType polynomType);

        Real state(const Path& path, Size t) const;
        Real operator()(const Path& path, Size t) const;

        std::vector<boost::function1<Real, Real> > basisSystem() const;

      protected:
        Real payoff(Real state) const;

        Real scalingValue_;
        const boost::shared_ptr<Payoff> payoff_;
        std::vector<boost::function1<Real, Real> > v_;
    };


    //! Monte Carlo American engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCAmericanEngine {
      public:
        MakeMCAmericanEngine(
                    const boost::shared_ptr<GeneralizedBlackScholesProcess>&);
        // named parameters
        MakeMCAmericanEngine& withSteps(Size steps);
        MakeMCAmericanEngine& withStepsPerYear(Size steps);
        MakeMCAmericanEngine& withSamples(Size samples);
        MakeMCAmericanEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCAmericanEngine& withMaxSamples(Size samples);
        MakeMCAmericanEngine& withSeed(BigNatural seed);
        MakeMCAmericanEngine& withAntitheticVariate(bool b = true);
        MakeMCAmericanEngine& withControlVariate(bool b = true);
        MakeMCAmericanEngine& withPolynomOrder(Size polynomOrer);
        MakeMCAmericanEngine& withBasisSystem(LsmBasisSystem::PolynomType);
        MakeMCAmericanEngine& withCalibrationSamples(Size calibrationSamples);

        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_, controlVariate_;
        Size steps_, stepsPerYear_;
        Size samples_, maxSamples_, calibrationSamples_;
        Real tolerance_;
        BigNatural seed_;
        Size polynomOrder_;
        LsmBasisSystem::PolynomType polynomType_;
    };

    template <class RNG, class S> inline
    MCAmericanEngine<RNG,S>::MCAmericanEngine(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
        Size timeSteps, Size timeStepsPerYear,
        bool antitheticVariate, bool controlVariate,
        Size requiredSamples, Real requiredTolerance,
        Size maxSamples,BigNatural seed,
        Size polynomOrder, LsmBasisSystem::PolynomType polynomType,
        Size nCalibrationSamples)
    : MCLongstaffSchwartzEngine<VanillaOption::engine,
                                SingleVariate,RNG,S>(
                                         process, timeSteps, timeStepsPerYear,
                                         false, antitheticVariate,
                                         controlVariate, requiredSamples,
                                         requiredTolerance, maxSamples,
                                         seed, nCalibrationSamples),
      polynomOrder_(polynomOrder),
      polynomType_(polynomType) {}

    template <class RNG, class S>
    inline void MCAmericanEngine<RNG,S>::calculate() const {
        MCLongstaffSchwartzEngine<VanillaOption::engine,
                                  SingleVariate,RNG,S>::calculate();
        if (this->controlVariate_) {
            // control variate might lead to small negative
            // option values for deep OTM options
            this->results_.value = std::max(0.0, this->results_.value);
        }
    }
        
    template <class RNG, class S>
    inline boost::shared_ptr<LongstaffSchwartzPathPricer<Path> >
    MCAmericanEngine<RNG,S>::lsmPathPricer() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        boost::shared_ptr<EarlyExercise> exercise =
            boost::dynamic_pointer_cast<EarlyExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");
        QL_REQUIRE(!exercise->payoffAtExpiry(),
                   "payoff at expiry not handled");

        boost::shared_ptr<AmericanPathPricer> earlyExercisePathPricer(
            new AmericanPathPricer(this->arguments_.payoff,
                                   polynomOrder_, polynomType_));

        return boost::shared_ptr<LongstaffSchwartzPathPricer<Path> > (
             new LongstaffSchwartzPathPricer<Path>(
                                      this->timeGrid(),
                                      earlyExercisePathPricer,
                                      *(process->riskFreeRate())));
    }

    template <class RNG, class S>
    inline boost::shared_ptr<PathPricer<Path> >
    MCAmericanEngine<RNG,S>::controlPathPricer() const {
        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "StrikedTypePayoff needed for control variate");

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        return boost::shared_ptr<PathPricer<Path> >(
            new EuropeanPathPricer(
                payoff->optionType(),
                payoff->strike(),
                process->riskFreeRate()->discount(this->timeGrid().back()))
            );
    }

    template <class RNG, class S>
    inline boost::shared_ptr<PricingEngine>
    MCAmericanEngine<RNG,S>::controlPricingEngine() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        return boost::shared_ptr<PricingEngine>(
                                         new AnalyticEuropeanEngine(process));
    }

    template <class RNG, class S>
    inline Real MCAmericanEngine<RNG,S>::controlVariateValue() const {
        boost::shared_ptr<PricingEngine> controlPE =
            this->controlPricingEngine();

        QL_REQUIRE(controlPE,
                   "engine does not provide "
                   "control variation pricing engine");

        VanillaOption::arguments* controlArguments =
            dynamic_cast<VanillaOption::arguments*>(controlPE->getArguments());
        *controlArguments = this->arguments_;
        controlArguments->exercise = boost::shared_ptr<Exercise>(
             new EuropeanExercise(this->arguments_.exercise->lastDate()));

        controlPE->calculate();

        const VanillaOption::results* controlResults =
            dynamic_cast<const VanillaOption::results*>(
                                                     controlPE->getResults());

        return controlResults->value;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>::MakeMCAmericanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process), antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      calibrationSamples_(2048),
      tolerance_(Null<Real>()), seed_(0),
      polynomOrder_(2),
      polynomType_ (LsmBasisSystem::Monomial) {}

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withPolynomOrder(Size polynomOrder) {
        polynomOrder_ = polynomOrder;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withBasisSystem(
                                    LsmBasisSystem::PolynomType polynomType) {
        polynomType_ = polynomType;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withCalibrationSamples(Size samples) {
        calibrationSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCAmericanEngine<RNG,S>&
    MakeMCAmericanEngine<RNG,S>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }


    template <class RNG, class S>
    inline
    MakeMCAmericanEngine<RNG,S>::operator boost::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return boost::shared_ptr<PricingEngine>(new
            MCAmericanEngine<RNG, S>(process_,
                                     steps_,
                                     stepsPerYear_,
                                     antithetic_,
                                     controlVariate_,
                                     samples_, tolerance_,
                                     maxSamples_,
                                     seed_,
                                     polynomOrder_,
                                     polynomType_,
                                     calibrationSamples_));
    }

}

#endif

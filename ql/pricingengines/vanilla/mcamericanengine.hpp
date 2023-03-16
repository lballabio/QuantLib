/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2016 Peter Caspers
 Copyright (C) 2022 Jonghee Lee

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
#include <ql/optional.hpp>
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
    template <class RNG = PseudoRandom, class S = Statistics,
              class RNG_Calibration = RNG>
    class MCAmericanEngine
        : public MCLongstaffSchwartzEngine<VanillaOption::engine,
                                           SingleVariate,RNG,S,RNG_Calibration> {
      public:
        MCAmericanEngine(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
                         Size timeSteps,
                         Size timeStepsPerYear,
                         bool antitheticVariate,
                         bool controlVariate,
                         Size requiredSamples,
                         Real requiredTolerance,
                         Size maxSamples,
                         BigNatural seed,
                         Size polynomialOrder,
                         LsmBasisSystem::PolynomialType polynomialType,
                         Size nCalibrationSamples = Null<Size>(),
                         const ext::optional<bool>& antitheticVariateCalibration = ext::nullopt(),
                         BigNatural seedCalibration = Null<Size>());

        void calculate() const override;

      protected:
        ext::shared_ptr<LongstaffSchwartzPathPricer<Path> > lsmPathPricer() const override;

        Real controlVariateValue() const override;
        ext::shared_ptr<PricingEngine> controlPricingEngine() const override;
        ext::shared_ptr<PathPricer<Path> > controlPathPricer() const override;

      private:
        const Size polynomialOrder_;
        const LsmBasisSystem::PolynomialType polynomialType_;
    };

    class AmericanPathPricer : public EarlyExercisePathPricer<Path>  {
      public:
        AmericanPathPricer(ext::shared_ptr<Payoff> payoff,
                           Size polynomialOrder,
                           LsmBasisSystem::PolynomialType polynomialType);

        Real state(const Path& path, Size t) const override;
        Real operator()(const Path& path, Size t) const override;

        std::vector<ext::function<Real(Real)> > basisSystem() const override;

      protected:
        Real payoff(Real state) const;

        Real scalingValue_ = 1.0;
        const ext::shared_ptr<Payoff> payoff_;
        std::vector<ext::function<Real(Real)> > v_;
    };


    //! Monte Carlo American engine factory
    template <class RNG = PseudoRandom, class S = Statistics,
              class RNG_Calibration = RNG>
    class MakeMCAmericanEngine {
      public:
        MakeMCAmericanEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        // named parameters
        MakeMCAmericanEngine& withSteps(Size steps);
        MakeMCAmericanEngine& withStepsPerYear(Size steps);
        MakeMCAmericanEngine& withSamples(Size samples);
        MakeMCAmericanEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCAmericanEngine& withMaxSamples(Size samples);
        MakeMCAmericanEngine& withSeed(BigNatural seed);
        MakeMCAmericanEngine& withAntitheticVariate(bool b = true);
        MakeMCAmericanEngine& withControlVariate(bool b = true);
        MakeMCAmericanEngine& withPolynomialOrder(Size polynomialOrder);
        MakeMCAmericanEngine& withBasisSystem(LsmBasisSystem::PolynomialType);
        MakeMCAmericanEngine& withCalibrationSamples(Size calibrationSamples);
        MakeMCAmericanEngine& withAntitheticVariateCalibration(bool b = true);
        MakeMCAmericanEngine& withSeedCalibration(BigNatural seed);

        /*! \deprecated Renamed to withPolynomialOrder.
                        Deprecated in version 1.26.
        */
        QL_DEPRECATED
        MakeMCAmericanEngine& withPolynomOrder(Size polynomialOrder);

        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_ = false, controlVariate_ = false;
        Size steps_, stepsPerYear_;
        Size samples_, maxSamples_, calibrationSamples_ = 2048;
        Real tolerance_;
        BigNatural seed_ = 0;
        Size polynomialOrder_ = 2;
        LsmBasisSystem::PolynomialType polynomialType_ = LsmBasisSystem::Monomial;
        ext::optional<bool> antitheticCalibration_;
        BigNatural seedCalibration_;
    };

    template <class RNG, class S, class RNG_Calibration>
    inline MCAmericanEngine<RNG, S, RNG_Calibration>::MCAmericanEngine(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed,
        Size polynomialOrder,
        LsmBasisSystem::PolynomialType polynomialType,
        Size nCalibrationSamples,
        const ext::optional<bool>& antitheticVariateCalibration,
        BigNatural seedCalibration)
    : MCLongstaffSchwartzEngine<VanillaOption::engine, SingleVariate, RNG, S, RNG_Calibration>(
          process,
          timeSteps,
          timeStepsPerYear,
          false,
          antitheticVariate,
          controlVariate,
          requiredSamples,
          requiredTolerance,
          maxSamples,
          seed,
          nCalibrationSamples,
          false,
          antitheticVariateCalibration,
          seedCalibration),
      polynomialOrder_(polynomialOrder), polynomialType_(polynomialType) {}

    template <class RNG, class S, class RNG_Calibration>
    inline void MCAmericanEngine<RNG, S, RNG_Calibration>::calculate() const {
        MCLongstaffSchwartzEngine<VanillaOption::engine, SingleVariate, RNG, S,
                                  RNG_Calibration>::calculate();
        if (this->controlVariate_) {
            // control variate might lead to small negative
            // option values for deep OTM options
            this->results_.value = std::max(0.0, this->results_.value);
        }
    }

    template <class RNG, class S, class RNG_Calibration>
    inline ext::shared_ptr<LongstaffSchwartzPathPricer<Path> >
    MCAmericanEngine<RNG, S, RNG_Calibration>::lsmPathPricer() const {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        ext::shared_ptr<EarlyExercise> exercise =
            ext::dynamic_pointer_cast<EarlyExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");
        QL_REQUIRE(!exercise->payoffAtExpiry(),
                   "payoff at expiry not handled");

        ext::shared_ptr<AmericanPathPricer> earlyExercisePathPricer(
            new AmericanPathPricer(this->arguments_.payoff,
                                   polynomialOrder_, polynomialType_));

        return ext::make_shared<LongstaffSchwartzPathPricer<Path> > (
             
                                      this->timeGrid(),
                                      earlyExercisePathPricer,
                                      *(process->riskFreeRate()));
    }

    template <class RNG, class S, class RNG_Calibration>
    inline ext::shared_ptr<PathPricer<Path> >
    MCAmericanEngine<RNG, S, RNG_Calibration>::controlPathPricer() const {
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "StrikedTypePayoff needed for control variate");

        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        return ext::shared_ptr<PathPricer<Path> >(
            new EuropeanPathPricer(
                payoff->optionType(),
                payoff->strike(),
                process->riskFreeRate()->discount(this->timeGrid().back()))
            );
    }

    template <class RNG, class S, class RNG_Calibration>
    inline ext::shared_ptr<PricingEngine>
    MCAmericanEngine<RNG, S, RNG_Calibration>::controlPricingEngine() const {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                              this->process_);
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        return ext::shared_ptr<PricingEngine>(
                                         new AnalyticEuropeanEngine(process));
    }

    template <class RNG, class S, class RNG_Calibration>
    inline Real
    MCAmericanEngine<RNG, S, RNG_Calibration>::controlVariateValue() const {
        ext::shared_ptr<PricingEngine> controlPE =
            this->controlPricingEngine();

        QL_REQUIRE(controlPE,
                   "engine does not provide "
                   "control variation pricing engine");

        auto* controlArguments = dynamic_cast<VanillaOption::arguments*>(controlPE->getArguments());
        *controlArguments = this->arguments_;
        controlArguments->exercise = ext::shared_ptr<Exercise>(
             new EuropeanExercise(this->arguments_.exercise->lastDate()));

        controlPE->calculate();

        const auto* controlResults =
            dynamic_cast<const VanillaOption::results*>(controlPE->getResults());

        return controlResults->value;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration>::MakeMCAmericanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()),
      antitheticCalibration_(ext::nullopt()), seedCalibration_(Null<Size>()) {}

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withPolynomialOrder(Size polynomialOrder) {
        polynomialOrder_ = polynomialOrder;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withPolynomOrder(Size polynomialOrder) {
        return withPolynomialOrder(polynomialOrder);
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withBasisSystem(LsmBasisSystem::PolynomialType polynomialType) {
        polynomialType_ = polynomialType;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withStepsPerYear(
        Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withAbsoluteTolerance(
        Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withMaxSamples(
        Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withCalibrationSamples(
        Size samples) {
        calibrationSamples_ = samples;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withAntitheticVariate(
        bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &MakeMCAmericanEngine<
        RNG, S, RNG_Calibration>::withAntitheticVariateCalibration(bool b) {
        antitheticCalibration_ = b;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration> &
    MakeMCAmericanEngine<RNG, S, RNG_Calibration>::withSeedCalibration(
        BigNatural seed) {
        seedCalibration_ = seed;
        return *this;
    }

    template <class RNG, class S, class RNG_Calibration>
    inline MakeMCAmericanEngine<RNG, S, RNG_Calibration>::
    operator ext::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(new
           MCAmericanEngine<RNG, S, RNG_Calibration>(process_,
                                     steps_,
                                     stepsPerYear_,
                                     antithetic_,
                                     controlVariate_,
                                     samples_, tolerance_,
                                     maxSamples_,
                                     seed_,
                                     polynomialOrder_,
                                     polynomialType_,
                                     calibrationSamples_,
                                     antitheticCalibration_,
                                     seedCalibration_));
    }

}

#endif

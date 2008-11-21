/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2006 Klaus Spanderen
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

/*! \file mcamericanbasketengine.hpp
    \brief Least-square Monte Carlo engines
*/

#ifndef quantlib_american_basket_montecarlo_engine_hpp
#define quantlib_american_basket_montecarlo_engine_hpp

#include <ql/qldefines.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/pricingengines/mclongstaffschwartzengine.hpp>
#include <ql/exercise.hpp>
#include <boost/function.hpp>

namespace QuantLib {

    //! least-square Monte Carlo engine
    /*! \warning This method is intrinsically weak for out-of-the-money
                 options.

        \ingroup basketengines
    */
    template <class RNG = PseudoRandom>
    class MCAmericanBasketEngine
        : public MCLongstaffSchwartzEngine<BasketOption::engine,
                                           MultiVariate,RNG> {
      public:
        MCAmericanBasketEngine(const boost::shared_ptr<StochasticProcessArray>&,
                               Size timeSteps,
                               Size timeStepsPerYear,
                               bool brownianBridge,
                               bool antitheticVariate,
                               Size requiredSamples,
                               Real requiredTolerance,
                               Size maxSamples,
                               BigNatural seed,
                               Size nCalibrationSamples = Null<Size>());
      protected:
        boost::shared_ptr<LongstaffSchwartzPathPricer<MultiPath> >
            lsmPathPricer() const;
    };


    //! Monte Carlo American basket-option engine factory
    template <class RNG = PseudoRandom>
    class MakeMCAmericanBasketEngine {
      public:
        MakeMCAmericanBasketEngine(
                            const boost::shared_ptr<StochasticProcessArray>&);
        // named parameters
        MakeMCAmericanBasketEngine& withSteps(Size steps);
        MakeMCAmericanBasketEngine& withStepsPerYear(Size steps);
        MakeMCAmericanBasketEngine& withBrownianBridge(bool b = true);
        MakeMCAmericanBasketEngine& withAntitheticVariate(bool b = true);
        MakeMCAmericanBasketEngine& withSamples(Size samples);
        MakeMCAmericanBasketEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCAmericanBasketEngine& withMaxSamples(Size samples);
        MakeMCAmericanBasketEngine& withSeed(BigNatural seed);
        MakeMCAmericanBasketEngine& withCalibrationSamples(Size samples);
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        boost::shared_ptr<StochasticProcessArray> process_;
        bool brownianBridge_, antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_, calibrationSamples_;
        Real tolerance_;
        BigNatural seed_;
    };


    class AmericanBasketPathPricer
        : public EarlyExercisePathPricer<MultiPath>  {
      public:
        AmericanBasketPathPricer(Size assetNumber,
                                 const boost::shared_ptr<Payoff>& payoff,
                                 Size polynomOrder = 2,
                                 LsmBasisSystem::PolynomType
                                 polynomType = LsmBasisSystem::Monomial);

        Array state(const MultiPath& path, Size t) const;
        Real operator()(const MultiPath& path, Size t) const;

        std::vector<boost::function1<Real, Array> > basisSystem() const;

      protected:
        Real payoff(const Array& state) const;

        const Size assetNumber_;
        const boost::shared_ptr<Payoff> payoff_;

        Real scalingValue_;
        std::vector<boost::function1<Real, Array> > v_;
    };

    template <class RNG> inline
    MCAmericanBasketEngine<RNG>::MCAmericanBasketEngine(
                   const boost::shared_ptr<StochasticProcessArray>& processes,
                   Size timeSteps,
                   Size timeStepsPerYear,
                   bool brownianBridge,
                   bool antitheticVariate,
                   Size requiredSamples,
                   Real requiredTolerance,
                   Size maxSamples,
                   BigNatural seed,
                   Size nCalibrationSamples)
        : MCLongstaffSchwartzEngine<BasketOption::engine,
                                    MultiVariate,RNG>(processes,
                                                      timeSteps,
                                                      timeStepsPerYear,
                                                      brownianBridge,
                                                      antitheticVariate,
                                                      false,
                                                      requiredSamples,
                                                      requiredTolerance,
                                                      maxSamples,
                                                      seed,
                                                      nCalibrationSamples) {}

    template <class RNG>
    inline boost::shared_ptr<LongstaffSchwartzPathPricer<MultiPath> >
    MCAmericanBasketEngine<RNG>::lsmPathPricer() const {

        boost::shared_ptr<StochasticProcessArray> processArray =
            boost::dynamic_pointer_cast<StochasticProcessArray>(
                                                              this->process_);
        QL_REQUIRE(processArray && processArray->size()>0,
                   "Stochastic process array required");

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
               processArray->process(0));
        QL_REQUIRE(process, "generalized Black-Scholes process required");

        boost::shared_ptr<EarlyExercise> exercise =
            boost::dynamic_pointer_cast<EarlyExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");
        QL_REQUIRE(!exercise->payoffAtExpiry(),
                   "payoff at expiry not handled");

        boost::shared_ptr<AmericanBasketPathPricer> earlyExercisePathPricer(
            new AmericanBasketPathPricer(processArray->size(),
                                         this->arguments_.payoff));

        return boost::shared_ptr<LongstaffSchwartzPathPricer<MultiPath> > (
             new LongstaffSchwartzPathPricer<MultiPath>(
                     this->timeGrid(),
                     earlyExercisePathPricer,
                     *(process->riskFreeRate())));
    }


    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>::MakeMCAmericanBasketEngine(
                     const boost::shared_ptr<StochasticProcessArray>& process)
    : process_(process), brownianBridge_(false), antithetic_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      calibrationSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG>
    inline MakeMCAmericanBasketEngine<RNG>&
    MakeMCAmericanBasketEngine<RNG>::withCalibrationSamples(Size samples) {
        calibrationSamples_ = samples;
        return *this;
    }

    template <class RNG>
    inline
    MakeMCAmericanBasketEngine<RNG>::operator
    boost::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return boost::shared_ptr<PricingEngine>(new
            MCAmericanBasketEngine<RNG>(process_,
                                        steps_,
                                        stepsPerYear_,
                                        brownianBridge_,
                                        antithetic_,
                                        samples_,
                                        tolerance_,
                                        maxSamples_,
                                        seed_,
                                        calibrationSamples_));
    }

}

#endif

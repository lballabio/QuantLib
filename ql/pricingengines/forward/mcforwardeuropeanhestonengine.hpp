/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file mcforwardeuropeanhestonengine.hpp
    \brief Monte Carlo engine for forward-starting strike-reset European options using Heston-like process
*/

#ifndef quantlib_mc_forward_european_heston_engine_hpp
#define quantlib_mc_forward_european_heston_engine_hpp

#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/forward/mcforwardvanillaengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    /*! References:

        Control Variate trade-off considerations discussed in pull request:
        https://github.com/lballabio/QuantLib/pull/948

        \ingroup forwardengines

        \test
        - Heston MC prices for a flat Heston process are
          compared to analytical BS prices with the same
          volatility for a range of moneynesses
        - Heston MC prices for a forward-starting option
          resetting at  t=0 are compared to semi-analytical
          Heston prices for a range of moneynesses
    */
    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MCForwardEuropeanHestonEngine
        : public MCForwardVanillaEngine<MultiVariate,RNG,S> {
      public:
        typedef
        typename MCForwardVanillaEngine<MultiVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename MCForwardVanillaEngine<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCForwardVanillaEngine<MultiVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCForwardEuropeanHestonEngine(
             const ext::shared_ptr<P>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed,
             bool controlVariate = false);
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const override;

        // Use the vanilla option running from t=0 to t=expiryTime with an analytic Heston pricer
        // as a control variate. Works well if resetTime small.
        ext::shared_ptr<path_pricer_type> controlPathPricer() const override;
        ext::shared_ptr<PricingEngine> controlPricingEngine() const override {
            ext::shared_ptr<P> process = ext::dynamic_pointer_cast<P>(this->process_);
            QL_REQUIRE(process, "Heston-like process required");

            ext::shared_ptr<HestonModel> hestonModel(new HestonModel(process));
            return ext::shared_ptr<PricingEngine>(new
                AnalyticHestonEngine(hestonModel));
        }
    };


    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MakeMCForwardEuropeanHestonEngine {
      public:
        explicit MakeMCForwardEuropeanHestonEngine(ext::shared_ptr<P> process);
        // named parameters
        MakeMCForwardEuropeanHestonEngine& withSteps(Size steps);
        MakeMCForwardEuropeanHestonEngine& withStepsPerYear(Size steps);
        MakeMCForwardEuropeanHestonEngine& withSamples(Size samples);
        MakeMCForwardEuropeanHestonEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCForwardEuropeanHestonEngine& withMaxSamples(Size samples);
        MakeMCForwardEuropeanHestonEngine& withSeed(BigNatural seed);
        MakeMCForwardEuropeanHestonEngine& withAntitheticVariate(bool b = true);
        MakeMCForwardEuropeanHestonEngine& withControlVariate(bool b = false);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<P> process_;
        bool antithetic_, controlVariate_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };


    class ForwardEuropeanHestonPathPricer : public PathPricer<MultiPath> {
      public:
        ForwardEuropeanHestonPathPricer(Option::Type type,
                                   Real moneyness,
                                   Size resetIndex,
                                   DiscountFactor discount);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        Option::Type type_;
        Real moneyness_;
        Size resetIndex_;
        DiscountFactor discount_;
    };


    // inline definitions

    template <class RNG, class S, class P>
    inline MCForwardEuropeanHestonEngine<RNG,S,P>::MCForwardEuropeanHestonEngine(
             const ext::shared_ptr<P>& process,
             Size timeSteps,
             Size timeStepsPerYear,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed,
             bool controlVariate)
    : MCForwardVanillaEngine<MultiVariate,RNG,S>(process,
                                                 timeSteps,
                                                 timeStepsPerYear,
                                                 false,
                                                 antitheticVariate,
                                                 requiredSamples,
                                                 requiredTolerance,
                                                 maxSamples,
                                                 seed,
                                                 controlVariate) {}


    template <class RNG, class S, class P>
    inline ext::shared_ptr<typename MCForwardEuropeanHestonEngine<RNG,S,P>::path_pricer_type>
        MCForwardEuropeanHestonEngine<RNG,S,P>::pathPricer() const {

        TimeGrid timeGrid = this->timeGrid();

        Time resetTime = this->process_->time(this->arguments_.resetDate);
        Size resetIndex = timeGrid.closestIndex(resetTime);

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        ext::shared_ptr<P> process =
            ext::dynamic_pointer_cast<P>(this->process_);
        QL_REQUIRE(process, "Heston like process required");

        return ext::shared_ptr<typename
            MCForwardEuropeanHestonEngine<RNG,S,P>::path_pricer_type>(
                new ForwardEuropeanHestonPathPricer(
                                        payoff->optionType(),
                                        this->arguments_.moneyness,
                                        resetIndex,
                                        process->riskFreeRate()->discount(
                                                   timeGrid.back())));
    }

    template <class RNG, class S, class P>
    inline ext::shared_ptr<typename MCForwardEuropeanHestonEngine<RNG,S,P>::path_pricer_type>
        MCForwardEuropeanHestonEngine<RNG,S,P>::controlPathPricer() const {

        // Control variate prices a vanilla option on the path, and compares to analytical Heston
        // vanilla price. First entry in TimeGrid is 0, so use the existing path pricer reset at 0
        Size resetIndex = 0;
        TimeGrid timeGrid = this->timeGrid();

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        ext::shared_ptr<P> process =
            ext::dynamic_pointer_cast<P>(this->process_);
        QL_REQUIRE(process, "Heston like process required");

        return ext::shared_ptr<typename
            MCForwardEuropeanHestonEngine<RNG,S,P>::path_pricer_type>(
                new ForwardEuropeanHestonPathPricer(
                                        payoff->optionType(),
                                        this->arguments_.moneyness,
                                        resetIndex,
                                        process->riskFreeRate()->discount(
                                                   timeGrid.back())));
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG, S, P>::MakeMCForwardEuropeanHestonEngine(
        ext::shared_ptr<P> process)
    : process_(std::move(process)), antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()), samples_(Null<Size>()),
      maxSamples_(Null<Size>()), tolerance_(Null<Real>()), seed_(0) {}

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withAbsoluteTolerance(
                                                             Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>&
    MakeMCForwardEuropeanHestonEngine<RNG,S,P>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCForwardEuropeanHestonEngine<RNG,S,P>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified - set EITHER steps OR stepsPerYear");
        return ext::shared_ptr<PricingEngine>(new
            MCForwardEuropeanHestonEngine<RNG,S,P>(process_,
                                                   steps_,
                                                   stepsPerYear_,
                                                   antithetic_,
                                                   samples_,
                                                   tolerance_,
                                                   maxSamples_,
                                                   seed_,
                                                   controlVariate_));
    }
}


#endif


#ifndef id_6eccc292203fbb5411572da73d1a92be
#define id_6eccc292203fbb5411572da73d1a92be
inline bool test_6eccc292203fbb5411572da73d1a92be(const int* i) {
    return i != nullptr;
}
#endif

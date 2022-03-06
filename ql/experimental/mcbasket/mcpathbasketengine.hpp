/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andrea Odetti

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

/*! \file mcpathbasketengine.hpp
    \brief Path-dependent European basket MC engine
*/

#ifndef quantlib_mc_path_basket_engine_hpp
#define quantlib_mc_path_basket_engine_hpp

#include <ql/experimental/mcbasket/pathmultiassetoption.hpp>
#include <ql/experimental/mcbasket/pathpayoff.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/termstructures/yield/impliedtermstructure.hpp>
#include <ql/timegrid.hpp>
#include <utility>

namespace QuantLib {

    //! Pricing engine for path dependent basket options using
    //  Monte Carlo simulation
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCPathBasketEngine  : public PathMultiAssetOption::engine,
                                public McSimulation<MultiVariate,RNG,S> {
      public:
        typedef typename McSimulation<MultiVariate,RNG,S>::path_generator_type
                                                          path_generator_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::path_pricer_type
                                                             path_pricer_type;
        typedef typename McSimulation<MultiVariate,RNG,S>::stats_type
                                                                   stats_type;
        // constructor
        MCPathBasketEngine(ext::shared_ptr<StochasticProcessArray>,
                           Size timeSteps,
                           Size timeStepsPerYear,
                           bool brownianBridge,
                           bool antitheticVariate,
                           bool controlVariate,
                           Size requiredSamples,
                           Real requiredTolerance,
                           Size maxSamples,
                           BigNatural seed);

        void calculate() const override {
            McSimulation<MultiVariate,RNG,S>::calculate(requiredTolerance_,
                                                        requiredSamples_,
                                                        maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
                results_.errorEstimate =
                    this->mcModel_->sampleAccumulator().errorEstimate();
        }

      protected:

        // McSimulation implementation
        TimeGrid timeGrid() const;
        ext::shared_ptr<path_generator_type> pathGenerator() const;
        ext::shared_ptr<path_pricer_type> pathPricer() const;

        // data members
        ext::shared_ptr<StochasticProcessArray> process_;
        Size timeSteps_;
        Size timeStepsPerYear_;
        Size requiredSamples_;
        Size maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    class EuropeanPathMultiPathPricer : public PathPricer<MultiPath> {
      public:
        EuropeanPathMultiPathPricer(ext::shared_ptr<PathPayoff>& payoff,
                                    std::vector<Size> timePositions,
                                    std::vector<Handle<YieldTermStructure> > forwardTermStructures,
                                    Array discounts);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        ext::shared_ptr<PathPayoff> payoff_;
        std::vector<Size> timePositions_;
        std::vector<Handle<YieldTermStructure> > forwardTermStructures_;
        Array discounts_;
    };


    // template definitions

    template <class RNG, class S>
    inline MCPathBasketEngine<RNG, S>::MCPathBasketEngine(
        ext::shared_ptr<StochasticProcessArray> process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<MultiVariate, RNG, S>(antitheticVariate, controlVariate),
      process_(std::move(process)), timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), brownianBridge_(brownianBridge), seed_(seed) {
        QL_REQUIRE(timeSteps != Null<Size>() ||
                   timeStepsPerYear != Null<Size>(),
                   "no time steps provided");
        QL_REQUIRE(timeSteps == Null<Size>() ||
                   timeStepsPerYear == Null<Size>(),
                   "both time steps and time steps per year were provided");
        QL_REQUIRE(timeSteps != 0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
        QL_REQUIRE(timeStepsPerYear != 0,
                   "timeStepsPerYear must be positive, "
                   << timeStepsPerYear << " not allowed");
        this->registerWith(process_);
    }


    template<class RNG, class S>
    inline
    ext::shared_ptr<typename MCPathBasketEngine<RNG,S>::path_generator_type>
    MCPathBasketEngine<RNG,S>::pathGenerator() const {

        ext::shared_ptr<PathPayoff> payoff = arguments_.payoff;
        QL_REQUIRE(payoff, "non-basket payoff given");

        Size numAssets = process_->size();

        TimeGrid grid = timeGrid();

        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(numAssets * (grid.size() - 1), seed_);

        return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(process_,
                                                 grid, gen, brownianBridge_));
    }

    template <class RNG, class S>
    inline TimeGrid MCPathBasketEngine<RNG,S>::timeGrid() const {
        const std::vector<Date> & fixings = this->arguments_.fixingDates;
        const Size numberOfFixings = fixings.size();

        std::vector<Time> fixingTimes(numberOfFixings);
        for (Size i = 0; i < numberOfFixings; ++i) {
            fixingTimes[i] =
                this->process_->time(fixings[i]);
        }

        const Size numberOfTimeSteps = timeSteps_ != Null<Size>() ? timeSteps_ : timeStepsPerYear_ * fixingTimes.back();

        return TimeGrid(fixingTimes.begin(), fixingTimes.end(), numberOfTimeSteps);
    }

    template <class RNG, class S>
    inline
    ext::shared_ptr<typename MCPathBasketEngine<RNG,S>::path_pricer_type>
    MCPathBasketEngine<RNG,S>::pathPricer() const {

        ext::shared_ptr<PathPayoff> payoff = arguments_.payoff;
        QL_REQUIRE(payoff, "non-basket payoff given");

        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                       process_->process(0));
        QL_REQUIRE(process, "Black-Scholes process required");

        const TimeGrid theTimeGrid = timeGrid();

        const std::vector<Time> & times = theTimeGrid.mandatoryTimes();
        const Size numberOfTimes = times.size();

        const std::vector<Date> & fixings = this->arguments_.fixingDates;

        QL_REQUIRE(fixings.size() == numberOfTimes, "Invalid dates/times");

        std::vector<Size> timePositions(numberOfTimes);
        Array discountFactors(numberOfTimes);
        std::vector<Handle<YieldTermStructure> > forwardTermStructures(numberOfTimes);

        const Handle<YieldTermStructure> & riskFreeRate = process->riskFreeRate();

        for (Size i = 0; i < numberOfTimes; ++i) {
            timePositions[i] = theTimeGrid.index(times[i]);
            discountFactors[i] = riskFreeRate->discount(times[i]);
            forwardTermStructures[i] = Handle<YieldTermStructure>(
                ext::make_shared<ImpliedTermStructure>(riskFreeRate,
                                                         fixings[i]));
        }

        return ext::shared_ptr<
            typename MCPathBasketEngine<RNG,S>::path_pricer_type>(
                        new EuropeanPathMultiPathPricer(payoff, timePositions,
                                                        forwardTermStructures,
                                                        discountFactors));
    }


    //! Monte Carlo Path Basket engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCPathBasketEngine {
      public:
        explicit MakeMCPathBasketEngine(ext::shared_ptr<StochasticProcessArray>);
        // named parameters
        MakeMCPathBasketEngine& withSteps(Size steps);
        MakeMCPathBasketEngine& withStepsPerYear(Size steps);
        MakeMCPathBasketEngine& withBrownianBridge(bool b = true);
        MakeMCPathBasketEngine& withSamples(Size samples);
        MakeMCPathBasketEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCPathBasketEngine& withMaxSamples(Size samples);
        MakeMCPathBasketEngine& withSeed(BigNatural seed);
        MakeMCPathBasketEngine& withAntitheticVariate(bool b = true);
        MakeMCPathBasketEngine& withControlVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<StochasticProcessArray> process_;
        bool antithetic_, controlVariate_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG, S>::MakeMCPathBasketEngine(
        ext::shared_ptr<StochasticProcessArray> process)
    : process_(std::move(process)), antithetic_(false), controlVariate_(false),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()), samples_(Null<Size>()),
      maxSamples_(Null<Size>()), tolerance_(Null<Real>()), brownianBridge_(false), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCPathBasketEngine<RNG,S>&
    MakeMCPathBasketEngine<RNG,S>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCPathBasketEngine<RNG,S>::operator ext::shared_ptr<PricingEngine>()
                                                                       const {
        return ext::shared_ptr<PricingEngine>(new
            MCPathBasketEngine<RNG,S>(process_,
                                      steps_,
                                      stepsPerYear_,
                                      brownianBridge_,
                                      antithetic_,
                                      controlVariate_,
                                      samples_,
                                      tolerance_,
                                      maxSamples_,
                                      seed_));
    }

}


#endif


#ifndef id_615ae888a816167fb5f4fa96e845d60a
#define id_615ae888a816167fb5f4fa96e845d60a
inline bool test_615ae888a816167fb5f4fa96e845d60a(const int* i) {
    return i != nullptr;
}
#endif

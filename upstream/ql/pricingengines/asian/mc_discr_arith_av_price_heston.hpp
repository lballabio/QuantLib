/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mc_discr_arith_av_price_heston.hpp
    \brief Heston MC engine for discrete arithmetic average price Asian
*/

#ifndef quantlib_mc_discrete_arithmetic_average_price_asian_heston_engine_hpp
#define quantlib_mc_discrete_arithmetic_average_price_asian_heston_engine_hpp

#include <ql/exercise.hpp>
#include <ql/experimental/asian/analytic_discr_geom_av_price_heston.hpp>
#include <ql/pricingengines/asian/mc_discr_geom_av_price_heston.hpp>
#include <ql/pricingengines/asian/mcdiscreteasianenginebase.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    //!  Heston MC pricing engine for discrete arithmetic average price Asian
    /*!
         By default, the MC discretization will use 1 time step per fixing date, but
         this can be controlled via timeSteps or timeStepsPerYear parameter, which
         will provide additional timesteps. The grid tries to space as evenly as it
         can and does not guarantee to match an exact number of steps, the precise
         grid used can be found in results_.additionalResults["TimeGrid"]

         Some performance metrics/graphs for the Control Variate are shown in the
         pull request: https://github.com/lballabio/QuantLib/pull/966

         \ingroup asianengines
         \test the correctness of the returned value is tested by
               reproducing results available in literature.
    */
    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MCDiscreteArithmeticAPHestonEngine
        : public MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S> {
      public:
        typedef typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::path_generator_type path_generator_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::path_pricer_type path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::stats_type stats_type;
        // constructor
        MCDiscreteArithmeticAPHestonEngine(
             const ext::shared_ptr<P>& process,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed,
             Size timeSteps = Null<Size>(),
             Size timeStepsPerYear = Null<Size>(),
             bool controlVariate = false);
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const override;

        // Use the experimental analytic geometric asian option as a control variate.
        ext::shared_ptr<path_pricer_type> controlPathPricer() const override;
        ext::shared_ptr<PricingEngine> controlPricingEngine() const override {
            ext::shared_ptr<P> process = ext::dynamic_pointer_cast<P>(this->process_);
            QL_REQUIRE(process, "Heston-like process required");

            return ext::shared_ptr<PricingEngine>(new
                AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(process));
        }
    };


    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MakeMCDiscreteArithmeticAPHestonEngine {
      public:
        explicit MakeMCDiscreteArithmeticAPHestonEngine(ext::shared_ptr<P> process);
        // named parameters
        MakeMCDiscreteArithmeticAPHestonEngine& withSamples(Size samples);
        MakeMCDiscreteArithmeticAPHestonEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDiscreteArithmeticAPHestonEngine& withMaxSamples(Size samples);
        MakeMCDiscreteArithmeticAPHestonEngine& withSeed(BigNatural seed);
        MakeMCDiscreteArithmeticAPHestonEngine& withAntitheticVariate(bool b = true);
        MakeMCDiscreteArithmeticAPHestonEngine& withSteps(Size steps);
        MakeMCDiscreteArithmeticAPHestonEngine& withStepsPerYear(Size steps);
        MakeMCDiscreteArithmeticAPHestonEngine& withControlVariate(bool b = false);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<P> process_;
        bool antithetic_ = false, controlVariate_ = false;
        Size samples_, maxSamples_, steps_, stepsPerYear_;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    class ArithmeticAPOHestonPathPricer : public PathPricer<MultiPath> {
      public:
        ArithmeticAPOHestonPathPricer(Option::Type type,
                                      Real strike,
                                      DiscountFactor discount,
                                      std::vector<Size> fixingIndices,
                                      Real runningSum = 0.0,
                                      Size pastFixings = 0);
        Real operator()(const MultiPath& multiPath) const override;

      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
        std::vector<Size> fixingIndices_;
        Real runningSum_;
        Size pastFixings_;
    };


    // inline definitions

    template <class RNG, class S, class P>
    inline
    MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::MCDiscreteArithmeticAPHestonEngine(
             const ext::shared_ptr<P>& process,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed,
             Size timeSteps,
             Size timeStepsPerYear,
             bool controlVariate)
    : MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>(process,
                                                             false,
                                                             antitheticVariate,
                                                             controlVariate,
                                                             requiredSamples,
                                                             requiredTolerance,
                                                             maxSamples,
                                                             seed,
                                                             timeSteps,
                                                             timeStepsPerYear) {
        QL_REQUIRE(timeSteps == Null<Size>() || timeStepsPerYear == Null<Size>(),
                   "both time steps and time steps per year were provided");
    }

    template <class RNG, class S, class P>
    inline ext::shared_ptr<
            typename MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::path_pricer_type>
        MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::pathPricer() const {

        // Keep track of the fixing indices, the path pricer will need to sum only these
        TimeGrid timeGrid = this->timeGrid();
        std::vector<Time> fixingTimes = timeGrid.mandatoryTimes();
        std::vector<Size> fixingIndexes;
        fixingIndexes.reserve(fixingTimes.size());
        for (Real fixingTime : fixingTimes) {
            fixingIndexes.push_back(timeGrid.closestIndex(fixingTime));
        }

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
            MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::path_pricer_type>(
                new ArithmeticAPOHestonPathPricer(
                    payoff->optionType(),
                    payoff->strike(),
                    process->riskFreeRate()->discount(exercise->lastDate()),
                    fixingIndexes,
                    this->arguments_.runningAccumulator,
                    this->arguments_.pastFixings));
    }

    template <class RNG, class S, class P>
    inline ext::shared_ptr<
            typename MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::path_pricer_type>
        MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::controlPathPricer() const {

        // Keep track of the fixing indices, the path pricer will need to prod only these
        TimeGrid timeGrid = this->timeGrid();
        std::vector<Time> fixingTimes = timeGrid.mandatoryTimes();
        std::vector<Size> fixingIndexes;
        fixingIndexes.reserve(fixingTimes.size());
        for (Real fixingTime : fixingTimes) {
            fixingIndexes.push_back(timeGrid.closestIndex(fixingTime));
        }

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

        // TODO: Currently the analytic pricer does not support seasoned asian
        // options (coming soon). Once that is available, we will be able to
        // pass seasoning details to the path pricer (NB. NEED to pass them to
        // the analytic pricer as well in that case).

        return ext::shared_ptr<typename
            MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::path_pricer_type>(
                new GeometricAPOHestonPathPricer(
                    payoff->optionType(),
                    payoff->strike(),
                    process->riskFreeRate()->discount(exercise->lastDate()),
                    fixingIndexes));
    }

    template <class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG, S, P>::
        MakeMCDiscreteArithmeticAPHestonEngine(ext::shared_ptr<P> process)
    : process_(std::move(process)), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      steps_(Null<Size>()), stepsPerYear_(Null<Size>()), tolerance_(Null<Real>()) {}

    template<class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withAbsoluteTolerance(
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
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template<class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withSteps(Size steps) {
        QL_REQUIRE(stepsPerYear_ == Null<Size>(),
                   "number of steps per year already set");
        steps_ = steps;
        return *this;
    }

    template<class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withStepsPerYear(Size steps) {
        QL_REQUIRE(steps_ == Null<Size>(),
                   "number of steps already set");
        stepsPerYear_ = steps;
        return *this;
    }

    template<class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>&
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::operator ext::shared_ptr<PricingEngine>() const {
        return ext::shared_ptr<PricingEngine>(new
            MCDiscreteArithmeticAPHestonEngine<RNG,S,P>(process_,
                                                        antithetic_,
                                                        samples_,
                                                        tolerance_,
                                                        maxSamples_,
                                                        seed_,
                                                        steps_,
                                                        stepsPerYear_,
                                                        controlVariate_));
    }
}

#endif

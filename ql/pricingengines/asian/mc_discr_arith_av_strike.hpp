/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file mc_discr_arith_av_strike.hpp
    \brief Monte Carlo engine for discrete arithmetic average-strike Asian
*/

#ifndef quantlib_mc_discrete_arithmetic_average_strike_asian_engine_hpp
#define quantlib_mc_discrete_arithmetic_average_strike_asian_engine_hpp

#include <ql/pricingengines/asian/mcdiscreteasianengine.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //!  Monte Carlo pricing engine for discrete arithmetic average-strike Asian
    /*!  \ingroup asianengines */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteArithmeticASEngine
        : public MCDiscreteAveragingAsianEngine<RNG,S> {
      public:
        typedef
        typename MCDiscreteAveragingAsianEngine<RNG,S>::path_generator_type
            path_generator_type;
        typedef typename MCDiscreteAveragingAsianEngine<RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngine<RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteArithmeticASEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
      protected:
        boost::shared_ptr<path_pricer_type> pathPricer() const;
    };


    class ArithmeticASOPathPricer : public PathPricer<Path> {
      public:
        ArithmeticASOPathPricer(Option::Type type,
                                DiscountFactor discount,
                                Real runningSum = 0.0,
                                Size pastFixings = 0);
        Real operator()(const Path& path) const;
      private:
        Option::Type type_;
        DiscountFactor discount_;
        Real runningSum_;
        Size pastFixings_;
    };



    // inline definitions

    template <class RNG, class S>
    inline
    MCDiscreteArithmeticASEngine<RNG,S>::MCDiscreteArithmeticASEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : MCDiscreteAveragingAsianEngine<RNG,S>(process,
                                            brownianBridge,
                                            antitheticVariate,
                                            false,
                                            requiredSamples,
                                            requiredTolerance,
                                            maxSamples,
                                            seed) {}

    template <class RNG, class S>
    inline
    boost::shared_ptr<
               typename MCDiscreteArithmeticASEngine<RNG,S>::path_pricer_type>
    MCDiscreteArithmeticASEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<EuropeanExercise> exercise =
            boost::dynamic_pointer_cast<EuropeanExercise>(
                this->arguments_.exercise);
        QL_REQUIRE(exercise, "wrong exercise given");

        return boost::shared_ptr<typename
            MCDiscreteArithmeticASEngine<RNG,S>::path_pricer_type>(
                new ArithmeticASOPathPricer(
                    payoff->optionType(),
                    this->process_->riskFreeRate()->discount(
                                                        exercise->lastDate()),
                    this->arguments_.runningAccumulator,
                    this->arguments_.pastFixings));
    }



    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteArithmeticASEngine {
      public:
        MakeMCDiscreteArithmeticASEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process);
        // named parameters
        MakeMCDiscreteArithmeticASEngine& withBrownianBridge(bool b = true);
        MakeMCDiscreteArithmeticASEngine& withSamples(Size samples);
        MakeMCDiscreteArithmeticASEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDiscreteArithmeticASEngine& withMaxSamples(Size samples);
        MakeMCDiscreteArithmeticASEngine& withSeed(BigNatural seed);
        MakeMCDiscreteArithmeticASEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_;
        Size samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline
    MakeMCDiscreteArithmeticASEngine<RNG,S>::MakeMCDiscreteArithmeticASEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process), antithetic_(false),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), brownianBridge_(true), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withAbsoluteTolerance(
                                                             Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withBrownianBridge(bool b) {
        brownianBridge_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCDiscreteArithmeticASEngine<RNG,S>&
    MakeMCDiscreteArithmeticASEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCDiscreteArithmeticASEngine<RNG,S>::
    operator boost::shared_ptr<PricingEngine>() const {
        return boost::shared_ptr<PricingEngine>(
            new MCDiscreteArithmeticASEngine<RNG,S>(process_,
                                                    brownianBridge_,
                                                    antithetic_,
                                                    samples_, tolerance_,
                                                    maxSamples_,
                                                    seed_));
    }

}


#endif

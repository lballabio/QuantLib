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

/*! \file mc_discr_arith_av_price_heston.hpp
    \brief Heston MC engine for discrete arithmetic average price Asian
*/

#ifndef quantlib_mc_discrete_arithmetic_average_price_asian_heston_engine_hpp
#define quantlib_mc_discrete_arithmetic_average_price_asian_heston_engine_hpp

#include <ql/pricingengines/asian/mcdiscreteasianenginebase.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //!  Heston MC pricing engine for discrete arithmetic average price Asian
    /*!  \ingroup asianengines
         \test the correctness of the returned value is tested by
               reproducing results available in literature.
    */
    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MCDiscreteArithmeticAPHestonEngine
        : public MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S> {
      public:
        typedef
        typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteArithmeticAPHestonEngine(
             const ext::shared_ptr<P>& process,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const;
    };


    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MakeMCDiscreteArithmeticAPHestonEngine {
      public:
        explicit MakeMCDiscreteArithmeticAPHestonEngine(
            const ext::shared_ptr<P>& process);
        // named parameters
        MakeMCDiscreteArithmeticAPHestonEngine& withSamples(Size samples);
        MakeMCDiscreteArithmeticAPHestonEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCDiscreteArithmeticAPHestonEngine& withMaxSamples(Size samples);
        MakeMCDiscreteArithmeticAPHestonEngine& withSeed(BigNatural seed);
        MakeMCDiscreteArithmeticAPHestonEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<P> process_;
        bool antithetic_;
        Size samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_;
    };


    class ArithmeticAPOHestonPathPricer : public PathPricer<MultiPath> {
      public:
        ArithmeticAPOHestonPathPricer(Option::Type type,
                                      Real strike,
                                      DiscountFactor discount,
                                      Real runningSum = 0.0,
                                      Size pastFixings = 0);
        Real operator()(const MultiPath& multiPath) const;
      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
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
             BigNatural seed)
    : MCDiscreteAveragingAsianEngineBase<MultiVariate,RNG,S>(process,
                                                             false,
                                                             antitheticVariate,
                                                             false,
                                                             requiredSamples,
                                                             requiredTolerance,
                                                             maxSamples,
                                                             seed) {}

    template <class RNG, class S, class P>
    inline ext::shared_ptr<
            typename MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::path_pricer_type>
        MCDiscreteArithmeticAPHestonEngine<RNG,S,P>::pathPricer() const {

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
                    this->arguments_.runningAccumulator,
                    this->arguments_.pastFixings));
    }


    template <class RNG, class S, class P>
    inline
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::MakeMCDiscreteArithmeticAPHestonEngine(
             const ext::shared_ptr<P>& process)
    : process_(process), antithetic_(false),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), seed_(0) {}


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


    template <class RNG, class S, class P>
    inline
    MakeMCDiscreteArithmeticAPHestonEngine<RNG,S,P>::operator ext::shared_ptr<PricingEngine>()
                                                                      const {
        return ext::shared_ptr<PricingEngine>(new
            MCDiscreteArithmeticAPHestonEngine<RNG,S,P>(process_,
                                                        antithetic_,
                                                        samples_,
                                                        tolerance_,
                                                        maxSamples_,
                                                        seed_));
    }



}


#endif

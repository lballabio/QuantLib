/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
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

/*! \file replicatingvarianceswapengine.hpp
    \brief Replicating engine for variance swaps
*/

#ifndef quantlib_replicating_varianceswap_engine_hpp
#define quantlib_replicating_varianceswap_engine_hpp

#include <ql/instruments/varianceswap.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! Variance-swap pricing engine using replicating cost,
    /*! as described in Demeterfi, Derman, Kamal & Zou,
        "A Guide to Volatility and Variance Swaps", 1999

        \ingroup forwardengines

        \test returned fair variances verified against results from literature
    */
    class ReplicatingVarianceSwapEngine : public VarianceSwap::engine {
      public:
        typedef std::vector<Real> StrikesType;
        // constructor
        ReplicatingVarianceSwapEngine(
                 const Real dk = 5.0,
                 const StrikesType& callStrikes = StrikesType(),
                 const StrikesType& putStrikes = StrikesType());
        void calculate() const;
      protected:
        // helper methods
        void computeOptionWeights(const StrikesType&,
                                  const Option::Type) const;
        Real computeLogPayoff(const Real, const Real) const;
        Real computeReplicatingPortfolio() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Real underlying() const;
        Time residualTime() const;
      private:
        const Real dk_;
        const StrikesType callStrikes_;
        const StrikesType putStrikes_;
    };


    // inline definitions

    inline ReplicatingVarianceSwapEngine::ReplicatingVarianceSwapEngine(
                const Real dk,
                const StrikesType& callStrikes,
                const StrikesType& putStrikes)
    : dk_(dk), callStrikes_(callStrikes), putStrikes_(putStrikes) {

        QL_REQUIRE(!callStrikes.empty() && !putStrikes.empty(),
                   "no strike(s) given");
        QL_REQUIRE(*std::min_element(putStrikes.begin(),putStrikes.end())>0.0,
                   "min put strike must be positive");
        QL_REQUIRE(*std::min_element(callStrikes.begin(), callStrikes.end())==
                   *std::max_element(putStrikes.begin(), putStrikes.end()),
                   "min call and max put strikes differ");
    }


    inline void ReplicatingVarianceSwapEngine::computeOptionWeights(
                            const StrikesType& availStrikes,
                            const Option::Type type) const {
        if (availStrikes.empty()) return;
        StrikesType strikes = availStrikes;

        // add end-strike for piecewise approximation
        switch (type) {
          case Option::Call:
            std::sort(strikes.begin(), strikes.end());
            strikes.push_back(strikes.back() + dk_);
            break;
          case Option::Put:
            std::sort(strikes.begin(), strikes.end(), std::greater<Real>());
            strikes.push_back(std::max(strikes.back() - dk_, 0.0));
            break;
          default:
            QL_FAIL("invalid option type");
        }

        // remove duplicate strikes
        StrikesType::iterator last =
            std::unique(strikes.begin(), strikes.end());
        strikes.erase(last, strikes.end());

        // compute weights
        Real f = strikes.front();
        Real slope, prevSlope = 0.0;
        for (StrikesType::const_iterator k=strikes.begin();
             // added end-strike discarded
             k<strikes.end()-1;
             k++) {
            slope = std::fabs((computeLogPayoff(*(k+1), f) -
                               computeLogPayoff(*k, f))/
                              (*(k+1) - *k));
            boost::shared_ptr<StrikedTypePayoff> payoff(
                                            new PlainVanillaPayoff(type, *k));
            if ( k == strikes.begin() )
                results_.optionWeights.push_back(std::make_pair(payoff,slope));
            else
                results_.optionWeights.push_back(
                                   std::make_pair(payoff, slope - prevSlope));
            prevSlope = slope;
        }
    }


    inline Real ReplicatingVarianceSwapEngine::computeLogPayoff(
                         const Real strike,
                         const Real callPutStrikeBoundary) const {
        Real f = callPutStrikeBoundary;
        return (2.0/residualTime()) * (((strike - f)/f) - std::log(strike/f));
    }


    inline
    Real ReplicatingVarianceSwapEngine::computeReplicatingPortfolio() const {

        boost::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(arguments_.maturityDate));
        boost::shared_ptr<PricingEngine> optionEngine(
                    new AnalyticEuropeanEngine(arguments_.stochasticProcess));
        Real optionsValue = 0.0;

        for (results_.iterator=results_.optionWeights.begin();
             results_.iterator<results_.optionWeights.end();
             results_.iterator++) {
            boost::shared_ptr<StrikedTypePayoff> payoff =
                results_.iterator->first;
            EuropeanOption option(payoff, exercise);
            option.setPricingEngine(optionEngine);
            Real weight = results_.iterator->second;
            optionsValue += option.NPV() * weight;
        }

        Real f = results_.optionWeights.front().first->strike();
        return 2.0 * riskFreeRate() -
            2.0/residualTime() *
            (((underlying()/riskFreeDiscount() - f)/f) +
             std::log(f/underlying())) +
            optionsValue/riskFreeDiscount();
    }


     // calculate fair variance via replicating portfolio
    inline void ReplicatingVarianceSwapEngine::calculate() const {
        computeOptionWeights(callStrikes_, Option::Call);
        computeOptionWeights(putStrikes_, Option::Put);

        results_.fairVariance = computeReplicatingPortfolio();
    }


    inline Real ReplicatingVarianceSwapEngine::underlying() const {
        return arguments_.stochasticProcess->stateVariable()->value();
    }


    inline Time ReplicatingVarianceSwapEngine::residualTime() const {
        return arguments_.stochasticProcess->time(arguments_.maturityDate);
    }


    inline Rate ReplicatingVarianceSwapEngine::riskFreeRate() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                 NoFrequency, true);
    }


    inline
    DiscountFactor ReplicatingVarianceSwapEngine::riskFreeDiscount() const {
        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");
        return process->riskFreeRate()->discount(residualTime());
    }

}


#endif

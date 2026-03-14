/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file replicatingvarianceswapengine.hpp
    \brief Replicating engine for variance swaps
*/

#ifndef quantlib_replicating_varianceswap_engine_hpp
#define quantlib_replicating_varianceswap_engine_hpp

#include <ql/exercise.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/instruments/varianceswap.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    //! Variance-swap pricing engine using replicating cost,
    /*! as described in Demeterfi, Derman, Kamal & Zou,
        "A Guide to Volatility and Variance Swaps", 1999

        \ingroup forwardengines

        \test returned variances verified against results from literature
    */
    class ReplicatingVarianceSwapEngine : public VarianceSwap::engine {
      public:
        typedef std::vector<std::pair<
                   ext::shared_ptr<StrikedTypePayoff>, Real> > weights_type;
        // constructor
        ReplicatingVarianceSwapEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                                      Real dk = 5.0,
                                      const std::vector<Real>& callStrikes = std::vector<Real>(),
                                      const std::vector<Real>& putStrikes = std::vector<Real>());
        void calculate() const override;

      protected:
        // helper methods
        void computeOptionWeights(const std::vector<Real>&,
                                  Option::Type,
                                  weights_type& optionWeights) const;
        Real computeLogPayoff(Real, Real) const;
        Real computeReplicatingPortfolio(
                                     const weights_type& optionWeights) const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Real underlying() const;
        Time residualTime() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Real dk_;
        std::vector<Real> callStrikes_, putStrikes_;
    };


    // inline definitions

    inline ReplicatingVarianceSwapEngine::ReplicatingVarianceSwapEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Real dk,
        const std::vector<Real>& callStrikes,
        const std::vector<Real>& putStrikes)
    : process_(std::move(process)), dk_(dk), callStrikes_(callStrikes), putStrikes_(putStrikes) {

        QL_REQUIRE(process_, "no process given");
        QL_REQUIRE(!callStrikes.empty() && !putStrikes.empty(),
                   "no strike(s) given");
        QL_REQUIRE(*std::min_element(putStrikes.begin(),putStrikes.end())>0.0,
                   "min put strike must be positive");
        QL_REQUIRE(*std::min_element(callStrikes.begin(), callStrikes.end())==
                   *std::max_element(putStrikes.begin(), putStrikes.end()),
                   "min call and max put strikes differ");
    }


    inline void ReplicatingVarianceSwapEngine::computeOptionWeights(
                                    const std::vector<Real>& availStrikes,
                                    const Option::Type type,
                                    weights_type& optionWeights) const {
        if (availStrikes.empty())
            return;

        std::vector<Real> strikes = availStrikes;

        // add end-strike for piecewise approximation
        switch (type) {
          case Option::Call:
            std::sort(strikes.begin(), strikes.end());
            strikes.push_back(strikes.back() + dk_);
            break;
          case Option::Put:
            std::sort(strikes.begin(), strikes.end(), std::greater<>());
            strikes.push_back(std::max(strikes.back() - dk_, 0.0));
            break;
          default:
            QL_FAIL("invalid option type");
        }

        // remove duplicate strikes
        auto last = std::unique(strikes.begin(), strikes.end());
        strikes.erase(last, strikes.end());

        // compute weights
        Real f = strikes.front();
        Real slope, prevSlope = 0.0;




        for (auto k=strikes.begin();
             // added end-strike discarded
             k<strikes.end()-1;
             ++k) {
            slope = std::fabs((computeLogPayoff(*(k+1), f) -
                               computeLogPayoff(*k, f))/
                              (*(k+1) - *k));
            ext::shared_ptr<StrikedTypePayoff> payoff(
                                            new PlainVanillaPayoff(type, *k));
            if ( k == strikes.begin() )
                optionWeights.emplace_back(payoff,slope);
            else
                optionWeights.emplace_back(payoff, slope - prevSlope);
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
    Real ReplicatingVarianceSwapEngine::computeReplicatingPortfolio(
                                    const weights_type& optionWeights) const {

        ext::shared_ptr<Exercise> exercise(
                               new EuropeanExercise(arguments_.maturityDate));
        ext::shared_ptr<PricingEngine> optionEngine(
                                        new AnalyticEuropeanEngine(process_));
        Real optionsValue = 0.0;

        for (auto i = optionWeights.begin(); i < optionWeights.end(); ++i) {
            ext::shared_ptr<StrikedTypePayoff> payoff = i->first;
            EuropeanOption option(payoff, exercise);
            option.setPricingEngine(optionEngine);
            Real weight = i->second;
            optionsValue += option.NPV() * weight;
        }

        Real f = optionWeights.front().first->strike();
        return 2.0 * riskFreeRate() -
            2.0/residualTime() *
            (((underlying()/riskFreeDiscount() - f)/f) +
             std::log(f/underlying())) +
            optionsValue/riskFreeDiscount();
    }


     // calculate variance via replicating portfolio
    inline void ReplicatingVarianceSwapEngine::calculate() const {
        weights_type optionWeigths;
        computeOptionWeights(callStrikes_, Option::Call, optionWeigths);
        computeOptionWeights(putStrikes_, Option::Put, optionWeigths);

        results_.variance = computeReplicatingPortfolio(optionWeigths);

        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(arguments_.maturityDate);
        Real multiplier;
        switch (arguments_.position) {
          case Position::Long:
            multiplier = 1.0;
            break;
          case Position::Short:
            multiplier = -1.0;
            break;
          default:
            QL_FAIL("Unknown position");
        }
        results_.value = multiplier * riskFreeDiscount * arguments_.notional *
            (results_.variance - arguments_.strike);

        results_.additionalResults["optionWeights"] = optionWeigths;
    }


    inline Real ReplicatingVarianceSwapEngine::underlying() const {
        return process_->x0();
    }


    inline Time ReplicatingVarianceSwapEngine::residualTime() const {
        return process_->time(arguments_.maturityDate);
    }


    inline Rate ReplicatingVarianceSwapEngine::riskFreeRate() const {
        return process_->riskFreeRate()->zeroRate(residualTime(), Continuous,
                                                  NoFrequency, true);
    }


    inline
    DiscountFactor ReplicatingVarianceSwapEngine::riskFreeDiscount() const {
        return process_->riskFreeRate()->discount(residualTime());
    }

}


#endif

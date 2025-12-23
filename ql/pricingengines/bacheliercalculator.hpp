/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file bacheliercalculator.hpp
    \brief Bachelier-formula calculator class
*/

#ifndef quantlib_bacheliercalculator_hpp
#define quantlib_bacheliercalculator_hpp

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Bachelier calculator class
    class BachelierCalculator {
      private:
        class Calculator;
      public:
        BachelierCalculator(const ext::shared_ptr<StrikedTypePayoff>& payoff,
                           Real forward,
                           Real stdDev,
                           Real discount = 1.0);
        BachelierCalculator(Option::Type optionType,
                           Real strike,
                           Real forward,
                           Real stdDev,
                           Real discount = 1.0);
        ~BachelierCalculator() = default;

        Real value() const;

        /*! Sensitivity to change in the underlying forward price. */
        Real deltaForward() const;
        /*! Sensitivity to change in the underlying spot price. */
        Real delta(Real spot) const;

        /*! Sensitivity in percent to a percent change in the
            underlying forward price. */
        Real elasticityForward() const;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        Real elasticity(Real spot) const;

        /*! Second order derivative with respect to change in the
            underlying forward price. */
        Real gammaForward() const;
        /*! Second order derivative with respect to change in the
            underlying spot price. */
        Real gamma(Real spot) const;

        /*! Sensitivity to time to maturity. */
        Real theta(Real spot,
                          Time maturity) const;
        /*! Sensitivity to time to maturity per day,
            assuming 365 day per year. */
        Real thetaPerDay(Real spot,
                                Time maturity) const;

        /*! Sensitivity to volatility. */
        Real vega(Time maturity) const;

        /*! Sensitivity to discounting rate. */
        Real rho(Time maturity) const;

        /*! Sensitivity to dividend/growth rate. */
        Real dividendRho(Time maturity) const;

        /*! Probability of being in the money in the bond martingale
            measure, i.e. N(d).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmCashProbability() const;

        /*! Probability of being in the money in the asset martingale
            measure, i.e. N(d).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmAssetProbability() const;

        /*! Sensitivity to strike. */
        Real strikeSensitivity() const;

        /*! gamma w.r.t. strike. */
        Real strikeGamma() const;

        /*! Sensitivity of vega to spot (Vanna) */
        Real vanna(Time maturity) const;

        /*! Sensitivity of volga to volatility (Volga) */
        Real volga(Time maturity) const;

        Real alpha() const;
        Real beta() const;
        
      protected:
        void initialize(const ext::shared_ptr<StrikedTypePayoff>& p);
        
        //! Member variables
        Real strike_, forward_, stdDev_, discount_, variance_;
        Real d_;  // Single d parameter for Bachelier model
        Real alpha_, beta_, DalphaDd_, DbetaDd_;  // Simplified derivative names
        Real n_d_, cum_d_;  // Single normal distribution values
        Real x_, DxDs_, DxDstrike_;
    };

    // inline
    inline Real BachelierCalculator::thetaPerDay(Real spot,
                                               Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

    inline Real BachelierCalculator::itmCashProbability() const {
        // For Bachelier model:
        // Call ITM probability: P(F > K) = N(d) where d = (F-K)/σ
        // Put ITM probability:  P(F < K) = N(-d) = 1 - N(d) where d = (F-K)/σ
        
        if (alpha_ >= 0) { // Call option (alpha_ = N(d) >= 0)
            return cum_d_;  // N(d)
        } else { // Put option (alpha_ = N(d) - 1 < 0)
            return 1.0 - cum_d_;  // N(-d) = 1 - N(d)
        }
    }

    inline Real BachelierCalculator::itmAssetProbability() const {
        // In Bachelier model, asset probability is the same as cash probability
        // since there's no drift adjustment like in Black-Scholes
        // Call ITM probability: P(F > K) = N(d) where d = (F-K)/σ  
        // Put ITM probability:  P(F < K) = N(-d) = 1 - N(d) where d = (F-K)/σ
        
        if (alpha_ >= 0) { // Call option
            return cum_d_;  // N(d)
        } else { // Put option
            return 1.0 - cum_d_;  // N(-d) = 1 - N(d)
        }
    }

    inline Real BachelierCalculator::alpha() const {
        return alpha_;
    }

    inline Real BachelierCalculator::beta() const {
        return beta_;
    }

}

#endif

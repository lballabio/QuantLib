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
 <http://quantlib.org/license.shtml>.

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
    /*! \bug When the variance is null, division by zero occur during
             the calculation of delta, delta forward, gamma, gamma
             forward, rho, dividend rho, vega, and strike sensitivity.
    */
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
        virtual ~BachelierCalculator() = default;

        virtual Real value() const;

        /*! Sensitivity to change in the underlying forward price. */
        virtual Real deltaForward() const;
        /*! Sensitivity to change in the underlying spot price. */
        virtual Real delta(Real spot) const;

        /*! Sensitivity in percent to a percent change in the
            underlying forward price. */
        virtual Real elasticityForward() const;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        virtual Real elasticity(Real spot) const;

        /*! Second order derivative with respect to change in the
            underlying forward price. */
        virtual Real gammaForward() const;
        /*! Second order derivative with respect to change in the
            underlying spot price. */
        virtual Real gamma(Real spot) const;

        /*! Sensitivity to time to maturity. */
        virtual Real theta(Real spot,
                          Time maturity) const;
        /*! Sensitivity to time to maturity per day,
            assuming 365 day per year. */
        virtual Real thetaPerDay(Real spot,
                                Time maturity) const;

        /*! Sensitivity to volatility. */
        virtual Real vega(Time maturity) const;

        /*! Sensitivity to discounting rate. */
        virtual Real rho(Time maturity) const;

        /*! Sensitivity to dividend/growth rate. */
        virtual Real dividendRho(Time maturity) const;

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
        virtual Real strikeSensitivity() const;

        /*! gamma w.r.t. strike. */
        virtual Real strikeGamma() const;

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
        // For Bachelier model, this is the probability that F > K (call) or F < K (put)
        // which is N(d) for call, 1-N(d) = N(-d) for put
        // where d = (F-K)/σ
        return cum_d_;
    }

    inline Real BachelierCalculator::itmAssetProbability() const {
        // In Bachelier model, asset probability is the same as cash probability
        // since there's no drift adjustment like in Black-Scholes
        return cum_d_;
    }

    inline Real BachelierCalculator::alpha() const {
        return alpha_;
    }

    inline Real BachelierCalculator::beta() const {
        return beta_;
    }

}

#endif

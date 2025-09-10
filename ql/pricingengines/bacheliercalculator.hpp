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

#include <ql/pricingengines/diffusioncalculator.hpp>

namespace QuantLib {

    //! Bachelier calculator class
    /*! \bug When the variance is null, division by zero occur during
             the calculation of delta, delta forward, gamma, gamma
             forward, rho, dividend rho, vega, and strike sensitivity.
    */
    class BachelierCalculator : public DiffusionCalculator {
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
        ~BachelierCalculator() override = default;

        Real value() const override;

        /*! Sensitivity to change in the underlying forward price. */
        Real deltaForward() const override;
        /*! Sensitivity to change in the underlying spot price. */
        Real delta(Real spot) const override;

        /*! Sensitivity in percent to a percent change in the
            underlying forward price. */
        Real elasticityForward() const override;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        Real elasticity(Real spot) const override;

        /*! Second order derivative with respect to change in the
            underlying forward price. */
        Real gammaForward() const override;
        /*! Second order derivative with respect to change in the
            underlying spot price. */
        Real gamma(Real spot) const override;

        /*! Sensitivity to time to maturity. */
        Real theta(Real spot,
                  Time maturity) const override;

        /*! Sensitivity to volatility. */
        Real vega(Time maturity) const override;

        /*! Sensitivity to discounting rate. */
        Real rho(Time maturity) const override;

        /*! Sensitivity to dividend/growth rate. */
        Real dividendRho(Time maturity) const override;

        /*! Probability of being in the money in the bond martingale
            measure, i.e. N(d2).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmCashProbability() const override;

        /*! Probability of being in the money in the asset martingale
            measure, i.e. N(d1).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmAssetProbability() const override;

        /*! Sensitivity to strike. */
        Real strikeSensitivity() const override;

        /*! gamma w.r.t. strike. */
        Real strikeGamma() const override;

        Real alpha() const override;
        Real beta() const override;
      protected:
        void initialize(const ext::shared_ptr<StrikedTypePayoff>& p) override;
    };

    // inline
    inline Real BachelierCalculator::itmCashProbability() const {
        // For Bachelier model, this is the probability that F > K (call) or F < K (put)
        // which is N(d) for call, 1-N(d) = N(-d) for put
        // where d = (F-K)/?
        return cum_d1_;
    }

    inline Real BachelierCalculator::itmAssetProbability() const {
        // In Bachelier model, asset probability is the same as cash probability
        // since there's no drift adjustment like in Black-Scholes
        return cum_d1_;
    }

    inline Real BachelierCalculator::alpha() const {
        return alpha_;
    }

    inline Real BachelierCalculator::beta() const {
        return beta_;
    }

}

#endif

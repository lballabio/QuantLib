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

/*! \file blackcalculator.hpp
    \brief Black-formula calculator class
*/

#ifndef quantlib_blackcalculator_hpp
#define quantlib_blackcalculator_hpp

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Black 1976 calculator class
    /*! \bug When the variance is null, division by zero occur during
             the calculation of delta, delta forward, gamma, gamma
             forward, rho, dividend rho, vega, and strike sensitivity.
    */
    class BlackCalculator {
      private:
        class Calculator;
      public:
        BlackCalculator(const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount = 1.0);
        BlackCalculator(Option::Type optionType,
                        Real strike,
                        Real forward,
                        Real stdDev,
                        Real discount = 1.0);
        virtual ~BlackCalculator() = default;

        Real value() const;

        /*! Sensitivity to change in the underlying forward price. */
        Real deltaForward() const;
        /*! Sensitivity to change in the underlying spot price. */
        virtual Real delta(Real spot) const;

        /*! Sensitivity in percent to a percent change in the
            underlying forward price. */
        Real elasticityForward() const;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        virtual Real elasticity(Real spot) const;

        /*! Second order derivative with respect to change in the
            underlying forward price. */
        Real gammaForward() const;
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
        Real vega(Time maturity) const;

        /*! Sensitivity to discounting rate. */
        Real rho(Time maturity) const;

        /*! Sensitivity to dividend/growth rate. */
        Real dividendRho(Time maturity) const;

        /*! Probability of being in the money in the bond martingale
            measure, i.e. N(d2).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmCashProbability() const;

        /*! Probability of being in the money in the asset martingale
            measure, i.e. N(d1).
            It is a risk-neutral probability, not the real world one.
        */
        Real itmAssetProbability() const;

        /*! Sensitivity to strike. */
        Real strikeSensitivity() const;

        /*! gamma w.r.t. strike. */
        Real strikeGamma() const;

        Real alpha() const;
        Real beta() const;
      protected:
        void initialize(const ext::shared_ptr<StrikedTypePayoff>& p);
        Real strike_, forward_, stdDev_, discount_, variance_;
        Real d1_, d2_;
        Real alpha_, beta_, DalphaDd1_, DbetaDd2_;
        Real n_d1_, cum_d1_, n_d2_, cum_d2_;
        Real x_, DxDs_, DxDstrike_;
    };

    // inline
    inline Real BlackCalculator::thetaPerDay(Real spot,
                                             Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

    inline Real BlackCalculator::itmCashProbability() const {
        return cum_d2_;
    }

    inline Real BlackCalculator::itmAssetProbability() const {
        return cum_d1_;
    }

    inline Real BlackCalculator::alpha() const {
        return alpha_;
    }

    inline Real BlackCalculator::beta() const {
        return beta_;
    }

}

#endif

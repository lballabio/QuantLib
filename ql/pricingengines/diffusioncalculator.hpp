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

/*! \file diffusioncalculator.hpp
    \brief Base class for diffusion model calculator classes
*/

#ifndef quantlib_diffusioncalculator_hpp
#define quantlib_diffusioncalculator_hpp

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Base class for diffusion model calculators
    /*! This abstract base class provides the common interface and 
        functionality for different diffusion model calculators like 
        Black-Scholes and Bachelier models.
        
        \bug When the variance is null, division by zero occur during
             the calculation of delta, delta forward, gamma, gamma
             forward, rho, dividend rho, vega, and strike sensitivity.
    */
    class DiffusionCalculator {
      public:
        DiffusionCalculator(const ext::shared_ptr<StrikedTypePayoff>& payoff,
                           Real forward,
                           Real stdDev,
                           Real discount = 1.0);
        DiffusionCalculator(Option::Type optionType,
                           Real strike,
                           Real forward,
                           Real stdDev,
                           Real discount = 1.0);
        virtual ~DiffusionCalculator() = default;

        //! Option value
        virtual Real value() const = 0;

        /*! Sensitivity to change in the underlying forward price. */
        virtual Real deltaForward() const = 0;
        /*! Sensitivity to change in the underlying spot price. */
        virtual Real delta(Real spot) const = 0;

        /*! Sensitivity in percent to a percent change in the
            underlying forward price. */
        virtual Real elasticityForward() const = 0;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        virtual Real elasticity(Real spot) const = 0;

        /*! Second order derivative with respect to change in the
            underlying forward price. */
        virtual Real gammaForward() const = 0;
        /*! Second order derivative with respect to change in the
            underlying spot price. */
        virtual Real gamma(Real spot) const = 0;

        /*! Sensitivity to time to maturity. */
        virtual Real theta(Real spot,
                          Time maturity) const = 0;
        /*! Sensitivity to time to maturity per day,
            assuming 365 day per year. */
        virtual Real thetaPerDay(Real spot,
                                Time maturity) const;

        /*! Sensitivity to volatility. */
        virtual Real vega(Time maturity) const = 0;

        /*! Sensitivity to discounting rate. */
        virtual Real rho(Time maturity) const = 0;

        /*! Sensitivity to dividend/growth rate. */
        virtual Real dividendRho(Time maturity) const = 0;

        /*! Probability of being in the money in the bond martingale
            measure, i.e. N(d2).
            It is a risk-neutral probability, not the real world one.
        */
        virtual Real itmCashProbability() const = 0;

        /*! Probability of being in the money in the asset martingale
            measure, i.e. N(d1).
            It is a risk-neutral probability, not the real world one.
        */
        virtual Real itmAssetProbability() const = 0;

        /*! Sensitivity to strike. */
        virtual Real strikeSensitivity() const = 0;

        /*! gamma w.r.t. strike. */
        virtual Real strikeGamma() const = 0;

        virtual Real alpha() const = 0;
        virtual Real beta() const = 0;

      protected:
        //! Initialize the calculator with a payoff
        virtual void initialize(const ext::shared_ptr<StrikedTypePayoff>& p) = 0;
        
        //! Common member variables
        Real strike_, forward_, stdDev_, discount_, variance_;
        Real d1_, d2_;
        Real alpha_, beta_, DalphaDd1_, DbetaDd2_;
        Real n_d1_, cum_d1_, n_d2_, cum_d2_;
        Real x_, DxDs_, DxDstrike_;
    };

    // inline
    inline Real DiffusionCalculator::thetaPerDay(Real spot,
                                                Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

}

#endif
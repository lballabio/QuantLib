/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackformula.hpp
    \brief Black formula
*/

#ifndef quantlib_blackformula_hpp
#define quantlib_blackformula_hpp

#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    //! Black-formula calculator
    /*! \bug When the variance is null, division by zero occur during
             the calculation of delta, delta forward, gamma, gamma
             forward, rho, dividend rho, vega, and strike sensitivity.
    */
    class BlackFormula {
      public:
        BlackFormula(Real forward,
                     DiscountFactor discount,
                     Real variance,
                     const boost::shared_ptr<StrikedTypePayoff>& payoff);
        Real value() const;
        Real delta(Real spot) const;
        //! Sensitivity in percent to a percent movement in the underlying.
        Real elasticity(Real spot) const;
        Real gamma(Real spot) const;
        Real deltaForward() const;
        //! Sensitivity in percent to a percent movement in the forward price.
        Real elasticityForward() const;
        Real gammaForward() const;
        Real theta(Real spot, Time maturity) const;
        Real thetaPerDay(Real spot, Time maturity) const;
        Real vega(Time maturity) const;
        Real rho(Time maturity) const;
        Real dividendRho(Time maturity) const;
        /*! Probability of being in the money in the bond martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        Real itmCashProbability() const;
        /*! Probability of being in the money in the asset martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        Real itmAssetProbability() const;
        Real strikeSensitivity() const;
        Real alpha() const;
        Real beta() const;
      private:
        Real forward_, discount_, variance_;
        Real stdDev_, strike_;
        Real D1_, D2_, alpha_, beta_, DalphaDd1_, DbetaDd2_;
        Real cum_d1_, cum_d2_;
        Real X_, DXDs_, DXDstrike_;
    };


    // inline definitions

    inline Real BlackFormula::thetaPerDay(Real spot, Time maturity) const {
        return theta(spot, maturity)/365.0;
    }

    inline Real BlackFormula::itmCashProbability() const {
        return cum_d2_;
    }

    inline Real BlackFormula::itmAssetProbability() const {
        return cum_d1_;
    }

    inline Real BlackFormula::alpha() const {
        return alpha_;
    }

    inline Real BlackFormula::beta() const {
        return beta_;
    }

}


#endif

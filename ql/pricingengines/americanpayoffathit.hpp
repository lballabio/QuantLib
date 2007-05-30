/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file americanpayoffathit.hpp
    \brief Analytical formulae for american exercise with payoff at hit
*/

#ifndef quantlib_americanpayoffathit_h
#define quantlib_americanpayoffathit_h

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Analytic formula for American exercise payoff at-hit options
    /*! \todo calculate greeks */
    class AmericanPayoffAtHit {
      public:
        AmericanPayoffAtHit(
                          Real spot,
                          DiscountFactor discount,
                          DiscountFactor dividendDiscount,
                          Real variance,
                          const boost::shared_ptr<StrikedTypePayoff>& payoff);
        Real value() const;
        Real delta() const;
        Real gamma() const;
        Real rho(Time maturity) const;
      private:
        Real spot_;
        DiscountFactor discount_, dividendDiscount_;
        Real variance_;
        Volatility stdDev_;

        Real strike_, K_, DKDstrike_;

        Real mu_, lambda_, muPlusLambda_, muMinusLambda_, log_H_S_;

        Real D1_, D2_, cum_d1_, cum_d2_;

        Real alpha_, beta_, DalphaDd1_, DbetaDd2_;

        bool inTheMoney_;
        Real forward_, X_, DXDstrike_;
    };


    // inline definitions

    inline Real AmericanPayoffAtHit::value() const {
        return K_ * (forward_ * alpha_ + X_ * beta_);
    }

}


#endif

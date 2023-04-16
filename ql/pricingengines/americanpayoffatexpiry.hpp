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

/*! \file americanpayoffatexpiry.hpp
    \brief Analytical formulae for american exercise with payoff at expiry
*/

#ifndef quantlib_americanpayoffatexpiry_h
#define quantlib_americanpayoffatexpiry_h

#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Analytic formula for American exercise payoff at-expiry options
    /*! \todo calculate greeks */
    class AmericanPayoffAtExpiry {
      public:
        AmericanPayoffAtExpiry(
                          Real spot,
                          DiscountFactor discount,
                          DiscountFactor dividendDiscount,
                          Real variance,
                          const std::shared_ptr<StrikedTypePayoff>& payoff,
                          bool knock_in = true);
        Real value() const;
      private:
        Real spot_;
        DiscountFactor discount_, dividendDiscount_;
        Real variance_;

        Real forward_;
        Volatility stdDev_;

        Real strike_, K_;

        Real mu_, log_H_S_;

        Real D1_, D2_, cum_d1_, cum_d2_, n_d1_, n_d2_;

        bool inTheMoney_;
        Real Y_, X_;
        bool knock_in_;
    };


    // inline definitions

    inline Real AmericanPayoffAtExpiry::value() const {
        return discount_ * K_ * (X_ * cum_d1_ + Y_ * cum_d2_);
    }

}


#endif

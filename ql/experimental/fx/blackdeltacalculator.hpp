/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Dimitri Reiswich

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

/*! \file blackdeltacalculator.hpp
    \brief Black-Scholes formula delta calculator class
*/

#ifndef quantlib_black_delta_calculator_hpp
#define quantlib_black_delta_calculator_hpp

#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>

namespace QuantLib {

    //! Black delta calculator class
    /*! Class includes many operations needed for different applications
        in FX markets, which has special quoation mechanisms, since
        every price can be expressed in both numeraires.
    */
    class BlackDeltaCalculator {
      public:
        // A parsimonious constructor is chosen, which for example
        // doesn't need a strike. The reason for this is, that we'd
        // like this class to calculate deltas for different strikes
        // many times, e.g. in a numerical routine, which will be the
        // case in the smile setup procedure.
        BlackDeltaCalculator(Option::Type ot,
                             DeltaVolQuote::DeltaType dt,
                             Real spot,
                             DiscountFactor dDiscount,   // domestic discount
                             DiscountFactor fDiscount,   // foreign discount
                             Real stdDev);

        // Give strike, receive delta according to specified type
        Real deltaFromStrike(Real strike) const;
        // Give delta according to specified type, receive strike
        Real strikeFromDelta(Real delta) const;

        Real cumD1(Real strike) const;    // N(d1) or N(-d1)
        Real cumD2(Real strike) const;    // N(d2) or N(-d2)

        Real nD1(Real strike) const;      // n(d1)
        Real nD2(Real strike) const;      // n(d2)

        void setDeltaType(DeltaVolQuote::DeltaType dt);
        void setOptionType(Option::Type ot);

        // The following function can be calculated without an explicit strike
        Real atmStrike(DeltaVolQuote::AtmType atmT) const;

      private:
        // alternative delta type
        Real strikeFromDelta(Real delta, DeltaVolQuote::DeltaType dt) const;


        DeltaVolQuote::DeltaType dt_;
        Option::Type ot_;
        DiscountFactor dDiscount_, fDiscount_;

        Real stdDev_, spot_, forward_;
        Integer phi_;
        Real fExpPos_,fExpNeg_;
    };


    class BlackDeltaPremiumAdjustedSolverClass {
      public:
        BlackDeltaPremiumAdjustedSolverClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev,
                        Real delta);

        Real operator()(Real strike) const;

      private:
        BlackDeltaCalculator bdc_;
        Real delta_;
    };


    class BlackDeltaPremiumAdjustedMaxStrikeClass {
      public:
        BlackDeltaPremiumAdjustedMaxStrikeClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev);

        Real operator()(Real strike) const;

      private:
        BlackDeltaCalculator bdc_;
        Real stdDev_;
    };

}


#endif

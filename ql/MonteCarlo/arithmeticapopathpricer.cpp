
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file arithmeticapopathpricer.cpp
    \brief arithmetic average price option path pricer
*/

#include <ql/MonteCarlo/arithmeticapopathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        ArithmeticAPOPathPricer_old::ArithmeticAPOPathPricer_old(
          Option::Type type, double underlying, double strike,
          DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<Path>(discount, useAntitheticVariance),
          underlying_(underlying), payoff_(type, strike) {
            QL_REQUIRE(underlying>0.0,
                "ArithmeticAPOPathPricer_old: "
                "underlying less/equal zero not allowed");
            QL_REQUIRE(strike>0.0,
                "ArithmeticAPOPathPricer_old: "
                "strike less/equal zero not allowed");
        }

        double ArithmeticAPOPathPricer_old::operator()(const Path& path) const {

            Size n = path.size();
            QL_REQUIRE(n>0,
                "ArithmeticAPOPathPricer_old: the path cannot be empty");

            double price1 = underlying_;
            double averagePrice1 = 0.0;
            Size fixings = n;
            if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                averagePrice1 = price1;
                fixings = n+1;
            }
            Size i;
            for (i=0; i<n; i++) {
                price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                averagePrice1 += price1;
            }
            averagePrice1 = averagePrice1/fixings;

            if (useAntitheticVariance_) {
                double price2 = underlying_;
                double averagePrice2 = 0.0;

                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    averagePrice2 = price2;
                for (i=0; i<n; i++) {
                    price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                    averagePrice2 += price2;
                }
                averagePrice2 = averagePrice2/fixings;
                return discount_/2.0 *
                    (payoff_(averagePrice1)+payoff_(averagePrice2));
            } else
                return discount_ * payoff_(averagePrice1);
        }

    }

}

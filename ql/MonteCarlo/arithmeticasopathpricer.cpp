
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file arithmeticasopathpricer.cpp
    \brief arithmetic average strike option path pricer

    \fullpath
    ql/MonteCarlo/%arithmeticasopathpricer.cpp
*/

// $Id$

#include <ql/MonteCarlo/arithmeticasopathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        ArithmeticASOPathPricer_old::ArithmeticASOPathPricer_old(
          Option::Type type, double underlying,
          DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<Path>(discount, useAntitheticVariance), type_(type),
          underlying_(underlying) {
            QL_REQUIRE(underlying>0.0,
                "ArithmeticASOPathPricer_old: "
                "underlying less/equal zero not allowed");
        }

        double ArithmeticASOPathPricer_old::operator()(const Path& path) const {

            Size n = path.size();
            QL_REQUIRE(n>0,"ArithmeticASOPathPricer_old: the path cannot be empty");

            double price1 = underlying_;
            double averageStrike1 = 0.0;
            Size fixings = n;
            if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                averageStrike1 = price1;
                fixings = n+1;
            }
            Size i;
            for (i=0; i<n; i++) {
                price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                averageStrike1 += price1;
            }
            averageStrike1 = averageStrike1/fixings;

            if (useAntitheticVariance_) {
                double price2 = underlying_;
                double averageStrike2 = 0.0;

                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    averageStrike2 = price2;
                for (i=0; i<n; i++) {
                    price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                    averageStrike2 += price2;
                }
                averageStrike2 = averageStrike2/fixings;
                return discount_/2.0*(Payoff(type_, averageStrike1)(price1)
                    +Payoff(type_, averageStrike2)(price2));
            } else
                return discount_ * Payoff(type_, averageStrike1)(price1);
        }

    }

}

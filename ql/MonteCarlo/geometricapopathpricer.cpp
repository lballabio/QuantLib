
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

/*! \file geometricapopathpricer.cpp
    \brief path pricer for geometric average price option
*/

#include <ql/MonteCarlo/geometricapopathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    GeometricAPOPathPricer_old::GeometricAPOPathPricer_old(
                          Option::Type type, double underlying, double strike,
                          DiscountFactor discount, bool useAntitheticVariance)
    : PathPricer_old<Path>(discount, useAntitheticVariance),
      underlying_(underlying), payoff_(type, strike) {
        QL_REQUIRE(underlying>0.0,
                   "GeometricAPOPathPricer_old: "
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>0.0,
                   "GeometricAPOPathPricer_old: "
                   "strike less/equal zero not allowed");
    }

    double GeometricAPOPathPricer_old::operator()(const Path& path) const {

        Size n = path.size();
        QL_REQUIRE(n>0,"GeometricAPOPathPricer_old: the path cannot be empty");

        double geoLogDrift = 0.0, geoLogDiffusion = 0.0;
        Size i;
        for (i=0; i<n; i++) {
            geoLogDrift += (n-i)*path.drift()[i];
            geoLogDiffusion += (n-i)*path.diffusion()[i];
        }
        Size fixings = n;
        if (path.timeGrid().mandatoryTimes()[0]==0.0) {
            fixings = n+1;
        }
        double averagePrice1 = underlying_*
            QL_EXP((geoLogDrift+geoLogDiffusion)/fixings);

        if (useAntitheticVariance_) {
            double averagePrice2 = underlying_*
                QL_EXP((geoLogDrift-geoLogDiffusion)/fixings);
            return discount_ * 0.5 *
                (payoff_(averagePrice1) +payoff_(averagePrice2));
        } else {
            return discount_* payoff_(averagePrice1);
        }

    }

}

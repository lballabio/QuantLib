
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

/*! \file geometricasopathpricer.cpp
    \brief path pricer for geometric average strike option
*/

#include <ql/MonteCarlo/geometricasopathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        GeometricASOPathPricer_old::GeometricASOPathPricer_old(Option::Type type,
            double underlying,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<Path>(discount, useAntitheticVariance), type_(type),
          underlying_(underlying) {
            QL_REQUIRE(underlying>0.0,
                "GeometricASOPathPricer_old: "
                "underlying less/equal zero not allowed");
        }
        
        double GeometricASOPathPricer_old::operator()(const Path& path) const {

            Size n = path.size();
            QL_REQUIRE(n>0,"GeometricASOPathPricer_old: the path cannot be empty");

            double logDrift = 0.0, logDiffusion = 0.0;
            double geoLogDrift = 0.0, geoLogDiffusion = 0.0;
            Size i;
            for (i=0; i<n; i++) {
                logDrift += path.drift()[i];
                logDiffusion += path.diffusion()[i];
                geoLogDrift += (n-i)*path.drift()[i];
                geoLogDiffusion += (n-i)*path.diffusion()[i];
            }
            Size fixings = n;
            if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                fixings = n+1;
            }
            double averageStrike1 = underlying_*
                QL_EXP((geoLogDrift+geoLogDiffusion)/fixings);

            if (useAntitheticVariance_) {
                double averageStrike2 = underlying_*
                    QL_EXP((geoLogDrift-geoLogDiffusion)/fixings);
                return discount_* 0.5 *
                    (PlainPayoff(type_, averageStrike1)(underlying_ *
                                            QL_EXP(logDrift+logDiffusion))
                    +PlainPayoff(type_, averageStrike2)(underlying_ *
                                            QL_EXP(logDrift-logDiffusion)));
            } else
                return discount_*PlainPayoff(type_, averageStrike1)(underlying_ *
                                            QL_EXP(logDrift+logDiffusion));
        }

    }

}

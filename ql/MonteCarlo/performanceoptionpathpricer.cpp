
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file performanceoptionpathpricer.cpp
    \brief path pricer for performance options

    \fullpath
    ql/MonteCarlo/%performanceoptionpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/performanceoptionpathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        PerformanceOptionPathPricer_old::PerformanceOptionPathPricer_old(
            Option::Type type,
            double underlying, double moneyness,
            const std::vector<DiscountFactor>& discounts,
            bool useAntitheticVariance)
        : PathPricer_old<Path>(1.0, useAntitheticVariance),
          underlying_(underlying), discounts_(discounts), 
          payoff_(type, moneyness) {
            QL_REQUIRE(underlying>0.0,
                "PerformanceOptionPathPricer_old: "
                "underlying less/equal zero not allowed");
            QL_REQUIRE(moneyness>0.0,
                "PerformanceOptionPathPricer_old: "
                "moneyness less/equal zero not allowed");
        }

        double PerformanceOptionPathPricer_old::operator()(const Path& path) const{
            Size n = path.size();
            QL_REQUIRE(n>0,
                "PerformanceOptionPathPricer_old: at least one option is required");
            QL_REQUIRE(n==2,
                "PerformanceOptionPathPricer_old: only one option for the time"
                " being");
            QL_REQUIRE(n==discounts_.size(),
                "PerformanceOptionPathPricer_old: discounts/options mismatch");

            std::vector<double> result(n);
            std::vector<double> assetValue(n);
            double log_drift = path.drift()[0];
            double log_random = path.diffusion()[0];
            assetValue[0]  = underlying_ * QL_EXP(log_drift+log_random);

            if (useAntitheticVariance_) {
                std::vector<double> assetValue2(n);
                assetValue2[0] = underlying_ * QL_EXP(log_drift-log_random);
                // removing first option, it should be 0.5
                result[0] = 0.0;
                for (Size i = 1 ; i < n; i++) {
                    log_drift  += path.drift()[i];
                    log_random += path.diffusion()[i];
                    assetValue[i] =underlying_*QL_EXP(log_drift+log_random);
                    assetValue2[i]=underlying_*QL_EXP(log_drift-log_random);
                    result[i] = 0.5 * discounts_[i] *
                        (payoff_(assetValue [i]/assetValue [i-1])
                        +payoff_(assetValue2[i]/assetValue2[i-1]));
                }
            } else {
                // removing first option
                result[0] = 0.0;
                for (Size i = 1 ; i < n; i++) {
                    log_drift  += path.drift()[i];
                    log_random += path.diffusion()[i];
                    assetValue[i]  = underlying_ *
                        QL_EXP(log_drift+log_random);
                    result[i] = discounts_[i] *
                        payoff_(assetValue[i]/assetValue [i-1]);
                }
            }

            return result[1];
        }

    }

}


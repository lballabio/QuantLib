
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
/*! \file cliquetoptionpathpricer.cpp
    \brief path pricer for cliquet options

    \fullpath
    ql/MonteCarlo/%cliquetoptionpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/cliquetoptionpathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        CliquetOptionPathPricer::CliquetOptionPathPricer(Option::Type type,
          double underlying, double moneyness, double accruedCoupon,
          double lastFixing, double localCap, double localFloor,
          double globalCap, double globalFloor,
          const RelinkableHandle<TermStructure>& riskFreeTS)
        : PathPricer<Path>(riskFreeTS), type_(type),
          underlying_(underlying), moneyness_(moneyness),
          accruedCoupon_(accruedCoupon), lastFixing_(lastFixing),
          localCap_(localCap), localFloor_(localFloor),
          globalCap_(globalCap), globalFloor_(globalFloor) {
            if (accruedCoupon == Null<double>())
                accruedCoupon = 0.0;

            if (localCap == Null<double>())
                localCap = QL_MAX_DOUBLE;

            if (localFloor == Null<double>())
                localFloor = 0.0;

            if (globalCap == Null<double>())
                globalCap = QL_MAX_DOUBLE;

            if (globalFloor == Null<double>())
                globalFloor = 0.0;
        }

        double CliquetOptionPathPricer::operator()(const Path& path) const {
            double result = accruedCoupon_;
            double lastFixing = lastFixing_;
            double underlying = underlying_;
            const TimeGrid& pathTimes = path.timeGrid();
            const std::vector<Time>& fixingTimes = pathTimes.mandatoryTimes();
            Size k = 0;
            for (Size i=0; i<fixingTimes.size(); i++) {
                double logIncrement = 0.0;
                while (pathTimes[k]<fixingTimes[i]) {
                    underlying *= QL_EXP(path[k]);
                    k++;
                }
                // new fixing
                double payoff = ExercisePayoff(type_, underlying,
                    moneyness_*lastFixing)/lastFixing;
                payoff = QL_MAX(payoff, localFloor_);
                payoff = QL_MIN(payoff, localCap_);
                result += payoff;
                lastFixing = underlying;
            }
            result = QL_MAX(result, globalFloor_);
            result = QL_MIN(result, globalCap_);
            return result * riskFreeTS_->discount(pathTimes.back());
        }





        CliquetOptionPathPricer_old::CliquetOptionPathPricer_old(Option::Type type,
          double underlying, double moneyness,
          const std::vector<DiscountFactor>& discounts,
          bool useAntitheticVariance)
        : PathPricer_old<Path>(1.0, useAntitheticVariance), type_(type),
          underlying_(underlying), moneyness_(moneyness),
          discounts_(discounts) {
            QL_REQUIRE(underlying>0.0,
                "CliquetOptionPathPricer_old: "
                "underlying less/equal zero not allowed");
            QL_REQUIRE(moneyness>0.0,
                "CliquetOptionPathPricer_old: "
                "moneyness less/equal zero not allowed");
        }

        double CliquetOptionPathPricer_old::operator()(const Path& path) const {
            Size n = path.size();
            QL_REQUIRE(n>0,
                "CliquetOptionPathPricer_old: at least one option is required");
            QL_REQUIRE(n==2,
                "CliquetOptionPathPricer_old: only one option for the time being");
            QL_REQUIRE(n==discounts_.size(),
                "CliquetOptionPathPricer_old: discounts/options mismatch");

            std::vector<double> result(n);
            std::vector<double> assetValue(n);
            double log_drift = path.drift()[0];
            double log_random = path.diffusion()[0];
            assetValue[0]  = underlying_ * QL_EXP(log_drift+log_random);
            double dummyStrike = assetValue[0];

            if (useAntitheticVariance_) {
                std::vector<double> assetValue2(n);
                assetValue2[0] = underlying_ * QL_EXP(log_drift-log_random);
                // removing first option, it should be 0.5
                result[0] = 0.0 * discounts_[0] * (ExercisePayoff(type_,
                    assetValue [0], dummyStrike) +
                    ExercisePayoff(type_,
                    assetValue2[0], dummyStrike)
                    );
                for (Size i = 1 ; i < n; i++) {
                    log_drift  += path.drift()[i];
                    log_random += path.diffusion()[i];
                    assetValue[i]  = underlying_ *
                        QL_EXP(log_drift+log_random);
                    assetValue2[i] = underlying_ *
                        QL_EXP(log_drift-log_random);
                    result[i] = 0.5 * discounts_[i] * (ExercisePayoff(type_,
                        assetValue [i], assetValue [i-1] *moneyness_) +
                        ExercisePayoff(type_,
                        assetValue2[i], assetValue2[i-1] *moneyness_)
                        );
                }
            } else {
                // removing first option
                result[0] = 0.0 * discounts_[0] *
                    ExercisePayoff(type_,
                    assetValue [0], dummyStrike);
                for (Size i = 1 ; i < n; i++) {
                    log_drift  += path.drift()[i];
                    log_random += path.diffusion()[i];
                    assetValue[i]  = underlying_ *
                        QL_EXP(log_drift+log_random);
                    result[i] = discounts_[i] *
                        ExercisePayoff(type_,
                        assetValue [i], assetValue [i-1] *moneyness_);
                }
            }

            return result[1];
        }

    }

}


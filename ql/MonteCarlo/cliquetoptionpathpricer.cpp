
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
                accruedCoupon_ = 0.0;

            if (localCap == Null<double>())
                localCap_ = QL_MAX_DOUBLE;

            if (localFloor == Null<double>())
                localFloor_ = 0.0;

            if (globalCap == Null<double>())
                globalCap_ = QL_MAX_DOUBLE;

            if (globalFloor == Null<double>())
                globalFloor_ = 0.0;
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
                double payoff = 0.0;
                while (pathTimes[k]<fixingTimes[i]) {
                    underlying *= QL_EXP(path[k]);
                    k++;
                }
                // incorporate payoff
                if (lastFixing != Null<double>()) {
                    payoff = Payoff(type_, moneyness_*lastFixing)(underlying)/lastFixing;
                    payoff = QL_MAX(payoff, localFloor_);
                    payoff = QL_MIN(payoff, localCap_);
                    result += payoff;
                }
                // new fixing
                lastFixing = underlying;
            }
            result = QL_MAX(result, globalFloor_);
            result = QL_MIN(result, globalCap_);
            return result * riskFreeTS_->discount(pathTimes.back());
        }





        CliquetOptionPathPricer_old::CliquetOptionPathPricer_old(
            Option::Type type, double underlying, double moneyness,
            double accruedCoupon, double lastFixing, double localCap,
            double localFloor, double globalCap, double globalFloor,
            const std::vector<DiscountFactor>& discounts,
            bool redemptionOnly,
            bool useAntitheticVariance)
        : PathPricer_old<Path>(1.0, useAntitheticVariance), type_(type),
          underlying_(underlying), moneyness_(moneyness),
          accruedCoupon_(accruedCoupon), lastFixing_(lastFixing),
          localCap_(localCap), localFloor_(localFloor),
          globalCap_(globalCap), globalFloor_(globalFloor), discounts_(discounts),
          redemptionOnly_(redemptionOnly) {
            QL_REQUIRE(underlying>0.0,
                "MyCliquet: "
                "underlying less/equal zero not allowed");

            QL_REQUIRE(moneyness>0.0,
                "MyCliquet: "
                "moneyness less/equal zero not allowed");

            if (accruedCoupon == Null<double>())
                accruedCoupon_ = 0.0;

            if (localCap == Null<double>())
                localCap_ = QL_MAX_DOUBLE;

            if (localFloor == Null<double>())
                localFloor_ = 0.0;

            if (globalCap == Null<double>())
                globalCap_ = QL_MAX_DOUBLE;

            if (globalFloor == Null<double>())
                globalFloor_ = 0.0;
        }

        double CliquetOptionPathPricer_old::operator()(const Path& path) const {
            Size n = path.size();
            QL_REQUIRE(n>0, "the path cannot be empty");

            QL_REQUIRE(n==discounts_.size(), "discounts/options mismatch");

            double result, result2, lastFixing, underlying, payoff;

            // start the simulation
            lastFixing = lastFixing_;
            underlying = underlying_;
            if (redemptionOnly_)
                result = accruedCoupon_;
            else
                result = 0.0;

            Size i;
            // step by step using the discretization of the path
            for (i=0; i<n; i++) {
                underlying *= QL_EXP(path[i]);
                // incorporate payoff
                if (lastFixing != Null<double>()) {
                    payoff = Payoff(type_, moneyness_*lastFixing)(underlying)/lastFixing;
                    payoff = QL_MAX(payoff, localFloor_);
                    payoff = QL_MIN(payoff, localCap_);
                    if (redemptionOnly_)
                        result += payoff;
                    else
                        result += payoff * discounts_[i];
                }
                // new fixing
                lastFixing = underlying;
            }
            if (redemptionOnly_) {
                result = QL_MAX(result, globalFloor_);
                result = QL_MIN(result, globalCap_);
            }

            if (useAntitheticVariance_) {
                // start the antothetic simulation
                lastFixing = lastFixing_;
                underlying = underlying_;
                if (redemptionOnly_)
                    result2 = accruedCoupon_;
                else
                    result2 = 0.0;

                for (i=0; i<n; i++) {
                    underlying *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                    // incorporate payoff
                    if (lastFixing != Null<double>()) {
                        payoff = Payoff(type_, moneyness_*lastFixing)(underlying)/lastFixing;
                        payoff = QL_MAX(payoff, localFloor_);
                        payoff = QL_MIN(payoff, localCap_);
                        if (redemptionOnly_)
                            result2 += payoff;
                        else
                            result2 += payoff * discounts_[i];
                    }
                    // new fixing
                    lastFixing = underlying;
                }
                if (redemptionOnly_) {
                    result2 = QL_MAX(result2, globalFloor_);
                    result2 = QL_MIN(result2, globalCap_);
                    return discounts_.back()*(result+result2)/2.0;
                } else {
                    return (result+result2)/2.0;
                }
            } else {
                if (redemptionOnly_) {
                    return discounts_.back()*result;
                } else {
                    return result;
                }
            }
        }

    }

}


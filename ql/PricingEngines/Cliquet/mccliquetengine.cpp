/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*
#include <ql/PricingEngines/Cliquet/mccliquetengine.hpp>

namespace QuantLib {

    CliquetOptionPathPricer::CliquetOptionPathPricer(
        Option::Type type,
        Real underlying, Real moneyness, Real accruedCoupon,
        Real lastFixing, Real localCap, Real localFloor,
        Real globalCap, Real globalFloor,
        bool redemptionOnly,
        const Handle<TermStructure>& riskFreeTS)
    : PathPricer<Path>(riskFreeTS), type_(type),
      underlying_(underlying), moneyness_(moneyness),
      accruedCoupon_(accruedCoupon), lastFixing_(lastFixing),
      localCap_(localCap), localFloor_(localFloor),
      globalCap_(globalCap), globalFloor_(globalFloor),
      redemptionOnly_(redemptionOnly), riskFreeTS_(riskFreeTS) {

        QL_REQUIRE(underlying_>0.0,
                   "underlying must be greater than zero");

        QL_REQUIRE(moneyness_>0.0,
                   "moneyness must be greater than zero");

        if (accruedCoupon == Null<Real>())
            accruedCoupon_ = 0.0;

        if (localCap == Null<Real>())
            localCap_ = QL_MAX_REAL;

        if (localFloor == Null<Real>())
            localFloor_ = 0.0;

        QL_REQUIRE(localCap_>=localFloor_,
                   "localCap cannot be less then localFloor");

        if (globalCap == Null<Real>())
            globalCap_ = QL_MAX_REAL;

        if (globalFloor == Null<Real>())
            globalFloor_ = 0.0;

        QL_REQUIRE(globalCap_>=globalFloor_,
                   "globalCap cannot be less then globalFloor");

        if (!redemptionOnly_) {
            QL_REQUIRE(globalFloor_==0.0,
                       "globalFloor can be non zero only if "
                       "the option is redemption-only");
        }
    }

    Real CliquetOptionPathPricer::operator()(const Path& path) const {

        Real payoff, result;
        Real lastFixing = lastFixing_;
        Real underlying = underlying_;
        const TimeGrid& pathTimes = path.timeGrid();
        // fixingTimes.last() must be the maturity == payment date
        const std::vector<Time>& fixingTimes = pathTimes.mandatoryTimes();

        if (redemptionOnly_) {
            result = accruedCoupon_;
            Size k = 0;
            for (Size i=0; i<fixingTimes.size(); i++) {
                while (pathTimes[k]<fixingTimes[i]) {
                    underlying *= std::exp(path[k]);
                    k++;
                }
                // incorporate payoff
                if (lastFixing != Null<Real>()) {
                    payoff = PlainVanillaPayoff(type_,
                        moneyness_*lastFixing)(underlying) / lastFixing;
                    payoff = std::max(payoff, localFloor_);
                    payoff = std::min(payoff, localCap_);
                    result += payoff;
                    if (result>=globalCap_) {
                        result = globalCap_;
                        continue;
                    }
                }
                // new fixing
                lastFixing = underlying;
            } // end of for loop
            result = std::max(result, globalFloor_) *
                riskFreeTS_->discount(pathTimes.back());
            return result;
        } else {
            Real couponNominal = accruedCoupon_;
            result = 0.0;
            Size k = 0;
            for (Size i=0; i<fixingTimes.size(); i++) {
                while (pathTimes[k]<fixingTimes[i]) {
                    underlying *= std::exp(path[k]);
                    k++;
                }
                // incorporate payoff
                if (lastFixing != Null<Real>()) {
                    payoff = PlainVanillaPayoff(type_,
                        moneyness_*lastFixing)(underlying) / lastFixing;
                    payoff = std::max(payoff, localFloor_);
                    payoff = std::min(payoff, localCap_);
                    payoff = std::min(payoff, globalCap_-couponNominal);
                    couponNominal += payoff;
                    result += payoff * riskFreeTS_->discount(fixingTimes[i]);
                    if (couponNominal>=globalCap_) {
                        continue;
                    }
                }
                // new fixing
                lastFixing = underlying;
            } // end of for loop
            return result;
        }
    }

}

*/

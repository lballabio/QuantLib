/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

#include <ql/experimental/averageois/averageoiscouponpricer.hpp>
#include <cmath>

using std::vector;
using std::exp;
using std::pow;
using std::log;

namespace QuantLib {

    void ArithmeticAveragedOvernightIndexedCouponPricer::initialize(
                                            const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const OvernightIndexedCoupon*>(&coupon);
        QL_ENSURE(coupon_, "wrong coupon type");
    }

    Rate ArithmeticAveragedOvernightIndexedCouponPricer::swapletRate() const {

        ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());

        const vector<Date>& fixingDates = coupon_->fixingDates();
        const vector<Time>& dt = coupon_->dt();

        Size n = dt.size(),
            i = 0;

        Real accumulatedRate = 0.0;

        // already fixed part
        Date today = Settings::instance().evaluationDate();
        while (i < n && fixingDates[i] < today) {
            // rate must have been fixed
            Rate pastFixing = IndexManager::instance().getHistory(
                index->name())[fixingDates[i]];
            QL_REQUIRE(pastFixing != Null<Real>(),
                "Missing " << index->name() <<
                " fixing for " << fixingDates[i]);
            accumulatedRate += pastFixing*dt[i];
            ++i;
        }

        // today is a border case
        if (i < n && fixingDates[i] == today) {
            // might have been fixed
            try {
                Rate pastFixing = IndexManager::instance().getHistory(
                    index->name())[fixingDates[i]];
                if (pastFixing != Null<Real>()) {
                    accumulatedRate += pastFixing*dt[i];
                    ++i;
                }
                else {
                    ;   // fall through and forecast
                }
            }
            catch (Error&) {
                ;       // fall through and forecast
            }
        }

        /* forward part using telescopic property in order
        to avoid the evaluation of multiple forward fixings
        (approximation proposed by Katsumi Takada)*/
        if (byApprox_ && i < n) {
            Handle<YieldTermStructure> curve =
                index->forwardingTermStructure();
            QL_REQUIRE(!curve.empty(),
                "null term structure set to this instance of " <<
                index->name());

            const vector<Date>& dates = coupon_->valueDates();
            DiscountFactor startDiscount = curve->discount(dates[i]);
            DiscountFactor endDiscount = curve->discount(dates[n]);

            accumulatedRate += log(startDiscount / endDiscount) - 
                convAdj1(curve->timeFromReference(dates[i]),
                         curve->timeFromReference(dates[n])) -
                convAdj2(curve->timeFromReference(dates[i]),
                         curve->timeFromReference(dates[n]));
        }
        // otherwise
        else if (i < n){
            Handle<YieldTermStructure> curve =
                index->forwardingTermStructure();
            QL_REQUIRE(!curve.empty(),
                "null term structure set to this instance of " <<
                index->name());

            const vector<Date>& dates = coupon_->valueDates();
            Time te = curve->timeFromReference(dates[n]);
            while (i < n) {
                // forcast fixing
                Rate forecastFixing = index->fixing(fixingDates[i]);
                Time ti1 = curve->timeFromReference(dates[i]);
                Time ti2 = curve->timeFromReference(dates[i + 1]);
                /*convexity adjustment due to payment dalay of each
                overnight fixing, supposing an Hull-White short rate model*/
                Real convAdj = exp( 0.5*pow(vol_, 2.0) / pow(mrs_, 3.0)*
                    (exp(2 * mrs_*ti1) - 1)*
                    (exp(-mrs_*ti2) - exp(-mrs_*te))*
                    (exp(-mrs_*ti2) - exp(-mrs_*ti1)) );
                accumulatedRate += convAdj*(1 + forecastFixing*dt[i]) - 1;
                ++i;
            }
        }

        Rate rate = accumulatedRate / coupon_->accrualPeriod();
        return coupon_->gearing() * rate + coupon_->spread();
    }

    Real ArithmeticAveragedOvernightIndexedCouponPricer::convAdj1(
                                                    Time ts, Time te) const {
        return vol_ * vol_ / (4.0 * pow(mrs_, 3.0)) *
            (1.0 - exp(-2.0*mrs_*ts)) *
            pow((1.0 - exp(-mrs_*(te - ts))), 2.0);
    }

    Real ArithmeticAveragedOvernightIndexedCouponPricer::convAdj2(
                                                    Time ts, Time te) const {
        return vol_ * vol_ / (2.0 * pow(mrs_, 2.0)) * ((te - ts) -
            pow(1.0 - exp(-mrs_*(te - ts)), 2.0) / mrs_ -
            (1.0 - exp(-2.0*mrs_*(te - ts))) / (2.0 * mrs_));
    }

}


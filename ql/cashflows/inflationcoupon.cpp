/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    InflationCoupon::InflationCoupon(
                                    const Date& paymentDate,
                                    Real nominal,
                                    const Date& startDate,
                                    const Date& endDate,
                                    Natural fixingDays,
                                    const boost::shared_ptr<InflationIndex>& index,
                                    const Period& observationLag,
                                    const DayCounter& dayCounter,
                                    const Date& refPeriodStart,
                                    const Date& refPeriodEnd
                                    )
    : Coupon(paymentDate, nominal,
             startDate, endDate, refPeriodStart,refPeriodEnd),  // ref period is before lag
      index_(index), observationLag_(observationLag), dayCounter_(dayCounter),
      fixingDays_(fixingDays)
    {
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }


    void InflationCoupon::setPricer(const boost::shared_ptr<InflationCouponPricer>& pricer) {
        QL_REQUIRE(checkPricerImpl(pricer),"pricer given is wrong type");
        if (pricer_)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_)
            registerWith(pricer_);
        update();
    }


    Rate InflationCoupon::rate() const {
        QL_REQUIRE(pricer_, "pricer not set");
        // we know it is the correct type because checkPricerImpl checks on setting
        // in general pricer_ will be a derived class, as will *this on calling
        pricer_->initialize(*this);
        return pricer_->swapletRate();
    }


    Real InflationCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
            dayCounter().yearFraction(accrualStartDate_,
                                      std::min(d, accrualEndDate_),
                                      refPeriodStart_,
                                      refPeriodEnd_);
        }
    }


    Date InflationCoupon::fixingDate() const {

        // fixing calendar is usually the null calendar for inflation indices
        return index_->fixingCalendar().advance(refPeriodEnd_-observationLag_,
                        -static_cast<Integer>(fixingDays_), Days, ModifiedPreceding);
    }


    Real InflationCoupon::price(const Handle<YieldTermStructure>& discountingCurve) const {
        return amount() * discountingCurve->discount(date());
    }


    Rate InflationCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

}

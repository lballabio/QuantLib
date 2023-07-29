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

#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    InflationCoupon::InflationCoupon(const Date& paymentDate,
                                     Real nominal,
                                     const Date& startDate,
                                     const Date& endDate,
                                     Natural fixingDays,
                                     ext::shared_ptr<InflationIndex> index,
                                     const Period& observationLag,
                                     DayCounter dayCounter,
                                     const Date& refPeriodStart,
                                     const Date& refPeriodEnd,
                                     const Date& exCouponDate)
    : Coupon(paymentDate,
             nominal,
             startDate,
             endDate,
             refPeriodStart,
             refPeriodEnd,
             exCouponDate), // ref period is before lag
      index_(std::move(index)), observationLag_(observationLag), dayCounter_(std::move(dayCounter)),
      fixingDays_(fixingDays) {
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }


    void InflationCoupon::setPricer(const ext::shared_ptr<InflationCouponPricer>& pricer) {
        QL_REQUIRE(checkPricerImpl(pricer),"pricer given is wrong type");
        if (pricer_ != nullptr)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_ != nullptr)
            registerWith(pricer_);
        update();
    }


    Rate InflationCoupon::rate() const {
        calculate();
        return rate_;
    }

    void InflationCoupon::performCalculations() const {
        QL_REQUIRE(pricer_, "pricer not set");
        // we know it is the correct type because checkPricerImpl checks on setting
        // in general pricer_ will be a derived class, as will *this on calling
        pricer_->initialize(*this);
        rate_ = pricer_->swapletRate();
    }


    Real InflationCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() * accruedPeriod(d);
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

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2006, 2007 Cristina Duminuco
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
 Copyright (C) 2003 Nicolas Di Césaré

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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FloatingRateCoupon::FloatingRateCoupon(const Date& paymentDate,
                                           Real nominal,
                                           const Date& startDate,
                                           const Date& endDate,
                                           Natural fixingDays,
                                           const ext::shared_ptr<InterestRateIndex>& index,
                                           Real gearing,
                                           Spread spread,
                                           const Date& refPeriodStart,
                                           const Date& refPeriodEnd,
                                           DayCounter dayCounter,
                                           bool isInArrears,
                                           const Date& exCouponDate,
                                           const ext::shared_ptr<FloatingRateCouponPricer>& pricer)
    : Coupon(paymentDate, nominal, startDate, endDate, refPeriodStart, refPeriodEnd, exCouponDate),
      index_(index), dayCounter_(std::move(dayCounter)),
      fixingDays_(fixingDays == Null<Natural>() ? (index ? index->fixingDays() : 0) : fixingDays),
      gearing_(gearing), spread_(spread), isInArrears_(isInArrears), pricer_(pricer),
      immutablePricer_(pricer != nullptr) {
        QL_REQUIRE(index_, "no index provided");
        QL_REQUIRE(gearing_!=0, "Null gearing not allowed");

        if (dayCounter_.empty())
            dayCounter_ = index_->dayCounter();

        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    std::pair<bool, std::set<ext::shared_ptr<Observable>>>
    FloatingRateCoupon::allowsNotificationPassThrough() const {
        return std::make_pair(!immutablePricer_,
                              immutablePricer_ ? std::set<ext::shared_ptr<Observable>>() :
                                                 std::set<ext::shared_ptr<Observable>>({pricer_}));
    }

    void FloatingRateCoupon::setPricer(
                const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
        QL_REQUIRE(!immutablePricer_, "setPricer() can not be called since pricer is immutable");
        if (pricer_ != nullptr)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_ != nullptr)
            registerWith(pricer_);
        update();
    }

    Real FloatingRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            // out of coupon range
            return 0.0;
        } else {
            return nominal() * rate() * accruedPeriod(d);
        }
    }

    Date FloatingRateCoupon::fixingDate() const {
        // if isInArrears_ fix at the end of period
        Date refDate = isInArrears_ ? accrualEndDate_ : accrualStartDate_;
        return index_->fixingCalendar().advance(refDate,
            -static_cast<Integer>(fixingDays_), Days, Preceding);
    }

    Rate FloatingRateCoupon::rate() const {
        calculate();
        return rate_;
    }

    void FloatingRateCoupon::performCalculations() const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);
        rate_ = pricer_->swapletRate();
    }

    Real FloatingRateCoupon::price(const Handle<YieldTermStructure>& discountingCurve) const {
        return amount() * discountingCurve->discount(date());
    }

    Rate FloatingRateCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

}

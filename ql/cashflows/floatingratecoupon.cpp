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

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    FloatingRateCoupon::FloatingRateCoupon(
                            const Date& paymentDate,
                            Real nominal,
                            const Date& startDate,
                            const Date& endDate,
                            Natural fixingDays,
                            const boost::shared_ptr<InterestRateIndex>& index,
                            Real gearing,
                            Spread spread,
                            const Date& refPeriodStart,
                            const Date& refPeriodEnd,
                            const DayCounter& dayCounter,
                            bool isInArrears)
    : Coupon(paymentDate, nominal,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter),
      fixingDays_(fixingDays==Null<Natural>() ? index->fixingDays() : fixingDays),
      gearing_(gearing), spread_(spread),
      isInArrears_(isInArrears)
    {
        init();
        // if isInArrears_ fix at the end of period
        Date refDate = isInArrears_ ? accrualEndDate_ : accrualStartDate_;
        fixingDate_ = index_->fixingCalendar().advance(
            refDate, -static_cast<Integer>(fixingDays_), Days, Preceding);
    }

    FloatingRateCoupon::FloatingRateCoupon(const Date& paymentDate,
                           Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           const Date& fixingDate,
                           const boost::shared_ptr<InterestRateIndex>& index,
                           Real gearing,
                           Spread spread,
                           const Date& refPeriodStart,
                           const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           const boost::optional<bool> isInArrears)
    : Coupon(paymentDate, nominal, startDate, endDate, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter), fixingDays_(Null<Size>()),
      fixingDate_(fixingDate), gearing_(gearing), spread_(spread)
    {
        QL_REQUIRE(fixingDate <= paymentDate,
                   "FloatingRateCoupon::FloatingRateCoupon(): fixing date ("
                       << fixingDate << ") can not be after payment date ("
                       << paymentDate << ")");
        init();
        // if the in arrears flag is explicitly given, we take that,
        // otherwise we deduce whether the fixing is in arrears from
        // the gap between the fixing date and the accrual end date
        isInArrears_ = isInArrears
                           ? *isInArrears
                           : index_->fixingCalendar().businessDaysBetween(
                                 fixingDate_, accrualEndDate_) < 5;
    }

    void FloatingRateCoupon::init() {
        QL_REQUIRE(!close_enough(gearing_, 0.0), "Null gearing not allowed");

        if (dayCounter_.empty())
            dayCounter_ = index_->dayCounter();

        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    void FloatingRateCoupon::setPricer(
                const boost::shared_ptr<FloatingRateCouponPricer>& pricer) {
        if (pricer_)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_)
            registerWith(pricer_);
        update();
    }

    Real FloatingRateCoupon::accruedAmount(const Date& d) const {
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

    Natural FloatingRateCoupon::fixingDays() const {
        QL_REQUIRE(
            fixingDays_ != Null<Size>(),
            "FloatingRateCoupon::fixingDays(): fixing days not provided");
        return fixingDays_;
    }

    Date FloatingRateCoupon::fixingDate() const {
        return fixingDate_;
    }

    Rate FloatingRateCoupon::rate() const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);
        return pricer_->swapletRate();
    }

    Real FloatingRateCoupon::price(const Handle<YieldTermStructure>& discountingCurve) const {
        return amount() * discountingCurve->discount(date());
    }

    Rate FloatingRateCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

}

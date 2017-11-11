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
                            boost::variant<Natural, Date> fixingDelay,
                            const boost::shared_ptr<InterestRateIndex>& index,
                            Real gearing,
                            Spread spread,
                            const Date& refPeriodStart,
                            const Date& refPeriodEnd,
                            const DayCounter& dayCounter,
                            boost::optional<bool> isInArrears)
    : Coupon(paymentDate, nominal,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      index_(index), fixingDelay_(fixingDelay), dayCounter_(dayCounter),
      gearing_(gearing), spread_(spread),
      isInArrears_(isInArrears)
    {
        QL_REQUIRE(!close_enough(gearing_, 0.0), "Null gearing not allowed");
        if (fixingDelay_.which() == 0 &&
            boost::get<Natural>(fixingDelay_) == Null<Natural>())
            fixingDelay_ = index->fixingDays();

        if (dayCounter_.empty())
            dayCounter_ = index_->dayCounter();

        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());

        if (fixingDelay_.which() == 0) {
            if (isInArrears_ == boost::none)
                isInArrears_ = false;
            // if isInArrears_ fix at the end of period
            Date refDate = *isInArrears_ ? accrualEndDate_ : accrualStartDate_;
            fixingDate_ = index_->fixingCalendar().advance(
                refDate,
                -static_cast<Integer>(boost::get<Natural>(fixingDelay_)), Days,
                Preceding);
        } else {
            fixingDate_ = boost::get<Date>(fixingDelay_);
            QL_REQUIRE(fixingDate_ != Null<Date>(),
                       "FloatingRateCoupon::FloatingRateCoupon(): null fixing "
                       "date not allowed");
            QL_REQUIRE(fixingDate_ <= date(),
                       "FloatingRateCoupon::FloatingRateCouon(): fixing date ("
                           << fixingDate_
                           << ") can not be later than payment date (" << date()
                           << ")");
        }
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
            hasFixingDays(),
            "FloatingRateCoupon::fixingDays(): fixing days not provided");
        return boost::get<Natural>(fixingDelay_);
    }

   bool FloatingRateCoupon::isInArrears() const {
       QL_REQUIRE(hasInArrears(),
                  "FloatingRateCoupon::isInArrears(): in arrears not provided");
       return *isInArrears_;
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

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    IborCoupon::IborCoupon(const Date& paymentDate, const Real nominal,
                           const Date& startDate, const Date& endDate,
                           const Natural fixingDays,
                           const boost::shared_ptr<InterestRateIndex>& index,
                           const Real gearing, const Spread spread,
                           const Date& refPeriodStart, const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate, fixingDays,
                         index, gearing, spread, refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears){ }

    Rate IborCoupon::indexFixing() const {

    #ifdef QL_USE_INDEXED_COUPON
        return index_->fixing(fixingDate());
    #else
        if (isInArrears()) {
            return index_->fixing(fixingDate());
        } else {
            Handle<YieldTermStructure> termStructure = index_->termStructure();
            QL_REQUIRE(!termStructure.empty(),
                       "null term structure set to par coupon");
            Date today = Settings::instance().evaluationDate();
            Date fixing_date = fixingDate();
            if (fixing_date < today) {
                // must have been fixed
                Rate pastFixing = IndexManager::instance().getHistory(
                                                     index_->name())[fixing_date];
                QL_REQUIRE(pastFixing != Null<Real>(),
                           "Missing " << index_->name()
                           << " fixing for " << fixing_date);
                return pastFixing;
            }
            if (fixing_date == today) {
                // might have been fixed
                try {
                    Rate pastFixing = IndexManager::instance().getHistory(
                                                     index_->name())[fixing_date];
                    if (pastFixing != Null<Real>())
                        return pastFixing;
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }

            // forecast: 1) startDiscount
            Date fixingValueDate = index_->fixingCalendar().advance(
                fixing_date, index_->fixingDays(), Days);
            DiscountFactor startDiscount = termStructure->discount(fixingValueDate);
            // forecast: 2) endDiscount
            Date nextFixingDate = index_->fixingCalendar().advance(
                accrualEndDate_, -static_cast<Integer>(fixingDays()), Days);
            Date nextFixingValueDate = index_->fixingCalendar().advance(
                nextFixingDate, index_->fixingDays(), Days);
            DiscountFactor endDiscount = termStructure->discount(nextFixingValueDate);
            // forecast: 3) spanningTime
            Time spanningTime = index_->dayCounter().yearFraction(
                fixingValueDate, nextFixingValueDate);
            // forecast: 4) implied fixing
            return (startDiscount/endDiscount-1.0)/spanningTime;
        }
    #endif

    }

    void IborCoupon::accept(AcyclicVisitor& v) {
        Visitor<IborCoupon>* v1 =
            dynamic_cast<Visitor<IborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}

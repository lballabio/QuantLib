/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

#include <ql/CashFlows/parcoupon.hpp>
#include <ql/Indexes/indexmanager.hpp>

namespace QuantLib {

    ParCoupon::ParCoupon(const Date& paymentDate, const Real nominal,
                         const Date& startDate, const Date& endDate,
                         const Integer fixingDays,
                         const boost::shared_ptr<Xibor>& index,
                         const Real gearing, const Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd,
                         const DayCounter& dayCounter)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, gearing, spread,
                         refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter) {
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    Rate ParCoupon::rate() const {
        boost::shared_ptr<YieldTermStructure> termStructure =
            index_->termStructure();
        QL_REQUIRE(termStructure,
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
            return gearing_*pastFixing+spread_;
        }
        if (fixing_date == today) {
            // might have been fixed
            try {
                Rate pastFixing = IndexManager::instance().getHistory(
                                                 index_->name())[fixing_date];
                if (pastFixing != Null<Real>())
                    return gearing_*pastFixing+spread_;
                else
                    ;   // fall through and forecast
            } catch (Error&) {
                ;       // fall through and forecast
            }
        }
        Date fixingValueDate = index_->calendar().advance(
                                 fixing_date, index_->settlementDays(), Days);
        DiscountFactor startDiscount =
            termStructure->discount(fixingValueDate);
        Date temp = index_->calendar().advance(accrualEndDate_,
                                               -fixingDays_, Days);
        DiscountFactor endDiscount =
            termStructure->discount(index_->calendar().advance(
                                       temp, index_->settlementDays(), Days));
        return gearing_*(startDiscount/endDiscount-1.0)/accrualPeriod() +
               spread_;
    }

    Rate ParCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

    Rate ParCoupon::convexityAdjustment(Rate f) const {
        if (gearing_ == 0.0)
            return 0.0;
        else
            return (rate()-spread_)/gearing_ - f;
    }

}


/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/parcoupon.hpp>
#include <ql/Indexes/indexmanager.hpp>

namespace QuantLib {

    ParCoupon::ParCoupon(Real nominal, const Date& paymentDate,
                         const boost::shared_ptr<Xibor>& index,
                         const Date& startDate, const Date& endDate,
                         Integer fixingDays, Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const DayCounter& dayCounter)
    : FloatingRateCoupon(nominal, paymentDate, startDate, endDate,
                         fixingDays, spread, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter) {
        registerWith(index_);
        registerWith(Settings::instance().evaluationDateGuard());
    }

    Real ParCoupon::amount() const {
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
                       "Missing " + index_->name() + " fixing for " +
                       DateFormatter::toString(fixing_date));
            return (pastFixing+spread_)*accrualPeriod()*nominal();
        }
        if (fixing_date == today) {
            // might have been fixed
            try {
                Rate pastFixing = IndexManager::instance().getHistory(
                                                 index_->name())[fixing_date];
                if (pastFixing != Null<Real>())
                    return (pastFixing+spread_)*accrualPeriod()*nominal();
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
        return ((startDiscount/endDiscount-1.0) +
                spread_*accrualPeriod()) * nominal();
    }

    Rate ParCoupon::indexFixing() const {
        DayCounter dayCount = index_->termStructure()->dayCounter();
        Date begin = index_->calendar().advance(
                                fixingDate(), index_->settlementDays(), Days);
        Date temp = index_->calendar().advance(accrualEndDate_,
                                               -fixingDays_, Days);
        Date end = index_->calendar().advance(
                                        temp, index_->settlementDays(), Days);
        return (amount()/nominal() - spread()*accrualPeriod()) /
            dayCount.yearFraction(begin,end);
    }

}


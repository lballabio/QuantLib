
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file floatingratecoupon.cpp
    \brief Coupon at par on a term structure

    \fullpath
    ql/CashFlows/%floatingratecoupon.cpp
*/

// $Id$

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Indexes/xibormanager.hpp>

namespace QuantLib {

    using Indexes::Xibor;
    using Indexes::XiborManager;

    namespace CashFlows {

        FloatingRateCoupon::FloatingRateCoupon(double nominal,
          const Date& paymentDate,
          const Handle<Xibor>& index,
          const Date& startDate, const Date& endDate,
          int fixingDays, Spread spread,
          const Date& refPeriodStart, const Date& refPeriodEnd)
        : Coupon(nominal, paymentDate, 
                 startDate, endDate, refPeriodStart, refPeriodEnd),
          index_(index), fixingDays_(fixingDays), spread_(spread) {
            registerWith(index_);
        }

        double FloatingRateCoupon::amount() const {
            Handle<TermStructure> termStructure = index_->termStructure();
            QL_REQUIRE(!termStructure.isNull(),
                       "null term structure set to par coupon");
            Date today = termStructure->todaysDate();
            Date fixingDate = index_->calendar().advance(
                accrualStartDate_, -fixingDays_, Days,
                Preceding);
            if (fixingDate < today) {
                // must have been fixed
                Rate pastFixing = XiborManager::getHistory(
                    index_->name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + index_->name() + " fixing for " +
                        DateFormatter::toString(fixingDate));
                return (pastFixing+spread_)*accrualPeriod()*nominal();
            }
            if (fixingDate == today) {
                // might have been fixed
                try {
                    Rate pastFixing = XiborManager::getHistory(
                        index_->name())[fixingDate];
                    if (pastFixing != Null<double>())
                        return (pastFixing+spread_) *
                            accrualPeriod() * nominal();
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }
            Date fixingValueDate = index_->calendar().advance(
                fixingDate, index_->settlementDays(), Days);
            DiscountFactor startDiscount =
                termStructure->discount(fixingValueDate);
            Date temp = index_->calendar().advance(accrualEndDate_,
                                                   -fixingDays_, Days);
            DiscountFactor endDiscount =
                termStructure->discount(
                    index_->calendar().advance(temp,
                        index_->settlementDays(), Days));
            return ((startDiscount/endDiscount-1.0) +
                spread_*accrualPeriod()) * nominal();
        }

    }

}


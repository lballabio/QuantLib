/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
          const Handle<Xibor>& index,
          const RelinkableHandle<TermStructure>& termStructure,
          const Date& startDate, const Date& endDate,
          int fixingDays, Spread spread,
          const Date& refPeriodStart, const Date& refPeriodEnd)
        : Coupon(nominal, index->calendar(),index->rollingConvention(),
              index->dayCounter(), startDate, endDate,
              refPeriodStart, refPeriodEnd),
          termStructure_(termStructure), index_(index),
          fixingDays_(fixingDays), spread_(spread) {
            termStructure_.registerObserver(this);
        }

        double FloatingRateCoupon::amount() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "null term structure set to par coupon");
            Date settlementDate = termStructure_->settlementDate();
            Date fixingDate = index_->calendar().advance(
                startDate_, -fixingDays_, Days,
                index_->rollingConvention());
            Date fixingValueDate = index_->calendar().advance(
                fixingDate, index_->settlementDays(), Days,
                index_->rollingConvention());
            if (fixingValueDate < settlementDate) {
                // must have been fixed
                Rate pastFixing = XiborManager::getHistory(
                    index_->name())[fixingDate];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + index_->name() + " fixing for " +
                        DateFormatter::toString(fixingDate));
                return (pastFixing+spread_)*accrualPeriod()*nominal();
            }
            if (fixingValueDate == settlementDate) {
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
            DiscountFactor startDiscount =
                termStructure_->discount(fixingValueDate);
            DiscountFactor endDiscount =
                termStructure_->discount(
                    index_->calendar().advance(endDate_,
                        index_->settlementDays()-fixingDays_, Days,
                        index_->rollingConvention()));
            return ((startDiscount/endDiscount-1.0) +
                spread_*accrualPeriod()) * nominal();
        }

    }

}


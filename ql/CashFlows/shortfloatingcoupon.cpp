
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
/*! \file shortfloatingcoupon.cpp
    \brief Short coupon at par on a term structure

    \fullpath
    ql/CashFlows/%shortfloatingcoupon.cpp
*/

// $Id$

#include <ql/CashFlows/shortfloatingcoupon.hpp>
#include <ql/Indexes/xibormanager.hpp>

namespace QuantLib {

    using Indexes::Xibor;

    namespace CashFlows {

        ShortFloatingRateCoupon::ShortFloatingRateCoupon(double nominal,
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

        double ShortFloatingRateCoupon::amount() const {
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
                // but we have no way to interpolate the fixing yet
                throw Error("short/long floating coupons not supported yet"
                    " (start = " + DateFormatter::toString(startDate_) +
                    ", end = " + DateFormatter::toString(endDate_) + ")");
            }
            if (fixingValueDate == settlementDate) {
                // might have been fixed
                // but we have no way to interpolate the fixing yet
                try {
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


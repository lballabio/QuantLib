
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/CashFlows/shortfloatingcoupon.hpp>
#include <ql/Indexes/xibormanager.hpp>

namespace QuantLib {

    ShortFloatingRateCoupon::ShortFloatingRateCoupon(
        double nominal, const Date& paymentDate,
        const Handle<Xibor>& index, const Date& startDate, 
        const Date& endDate, int fixingDays, Spread spread,
        const Date& refPeriodStart, const Date& refPeriodEnd)
    : ParCoupon(nominal,paymentDate,index,
                startDate,endDate,fixingDays,
                spread,refPeriodStart,refPeriodEnd) {}

    double ShortFloatingRateCoupon::amount() const {
        QL_REQUIRE(!IsNull(index()->termStructure()),
                   "null term structure set to par coupon");
        Date today = index()->termStructure()->todaysDate();
        Date fixing_date = fixingDate();
        QL_REQUIRE(fixing_date >= today,
                   // must have been fixed
                   // but we have no way to interpolate the fixing yet
                   "short/long floating coupons not supported yet"
                   " (start = " + 
                   DateFormatter::toString(accrualStartDate_) +
                   ", end = " + 
                   DateFormatter::toString(accrualEndDate_) + ")");
        return ParCoupon::amount();
    }

}



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
          const Date& paymentDate,
          const Handle<Xibor>& index,
          const Date& startDate, const Date& endDate,
          int fixingDays, Spread spread,
          const Date& refPeriodStart, const Date& refPeriodEnd)
        : FloatingRateCoupon(nominal,paymentDate,index,startDate,endDate,fixingDays,
                             spread,refPeriodStart,refPeriodEnd) {}

        double ShortFloatingRateCoupon::amount() const {
            QL_REQUIRE(!index()->termStructure().isNull(),
                "null term structure set to par coupon");
            Date settlementDate = index()->termStructure()->settlementDate();
            Date fixingDate = index()->calendar().advance(
                accrualStartDate_, -fixingDays(), Days,
                Preceding);
            Date fixingValueDate = index()->calendar().advance(
                fixingDate, index()->settlementDays(), Days,
                Following);
            QL_REQUIRE(fixingValueDate > settlementDate,
                       // must have been fixed
                       // but we have no way to interpolate the fixing yet
                       "short/long floating coupons not supported yet"
                       " (start = " + DateFormatter::toString(accrualStartDate_) +
                       ", end = " + DateFormatter::toString(accrualEndDate_) + ")");
            return FloatingRateCoupon::amount();
        }

    }

}


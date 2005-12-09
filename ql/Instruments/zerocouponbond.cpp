/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/Instruments/zerocouponbond.hpp>
#include <ql/CashFlows/simplecashflow.hpp>

namespace QuantLib {

    ZeroCouponBond::ZeroCouponBond(
                             const Date& issueDate,
                             const Date& maturityDate,
                             Integer settlementDays,
                             const DayCounter& dayCounter,
                             const Calendar& calendar,
                             BusinessDayConvention convention,
                             Real redemption,
                             const Handle<YieldTermStructure>& discountCurve)
    : Bond(dayCounter, calendar, convention, settlementDays, discountCurve) {

        issueDate_ = datedDate_ = issueDate;
        maturityDate_ = calendar.adjust(maturityDate,convention);
        frequency_ = Once;

        redemption_ = boost::shared_ptr<CashFlow>(
                                new SimpleCashFlow(redemption,maturityDate_));

        cashFlows_ = std::vector<boost::shared_ptr<CashFlow> >();
    }

}


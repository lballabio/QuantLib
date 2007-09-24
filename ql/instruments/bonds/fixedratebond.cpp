/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    FixedRateBond::FixedRateBond(Natural settlementDays,
                                 Real faceAmount,
                                 const Schedule& schedule,
                                 const std::vector<Rate>& coupons,
                                 const DayCounter& accrualDayCounter,
                                 BusinessDayConvention paymentConvention,
                                 Real redemption,
                                 const Date& issueDate)
    : Bond(settlementDays, schedule.calendar(), faceAmount, schedule.endDate(), issueDate) {

        cashflows_ = FixedRateLeg(schedule,accrualDayCounter)
            .withNotionals(faceAmount_)
            .withCouponRates(coupons)
            .withPaymentAdjustment(paymentConvention);

        Date redemptionDate = calendar_.adjust(maturityDate_,
                                               paymentConvention);
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(faceAmount_*redemption/100.0, redemptionDate)));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
    }

    FixedRateBond::FixedRateBond(Natural settlementDays,
                                 const Calendar& calendar,
                                 Real faceAmount,
                                 const Date& startDate,
                                 const Date& maturityDate,
                                 const Period& tenor,
                                 const std::vector<Rate>& coupons,
                                 const DayCounter& accrualDayCounter,
                                 BusinessDayConvention accrualConvention,
                                 BusinessDayConvention paymentConvention,
                                 Real redemption,
                                 const Date& issueDate,
                                 const Date& stubDate,
                                 bool fromEnd)
    : Bond(settlementDays, calendar, faceAmount, maturityDate, issueDate) {

        maturityDate_     = maturityDate;

        Date firstDate = (fromEnd ? Date() : stubDate);
        Date nextToLastDate = (fromEnd ? stubDate : Date());
        Schedule schedule(startDate, maturityDate_, tenor,
                          calendar_, accrualConvention, accrualConvention,
                          fromEnd, false, firstDate, nextToLastDate);

        cashflows_ = FixedRateLeg(schedule, accrualDayCounter)
            .withNotionals(faceAmount_)
            .withCouponRates(coupons)
            .withPaymentAdjustment(paymentConvention);

        Date redemptionDate = calendar_.adjust(maturityDate_,
                                               paymentConvention);
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(faceAmount_*redemption/100.0, redemptionDate)));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
    }

}

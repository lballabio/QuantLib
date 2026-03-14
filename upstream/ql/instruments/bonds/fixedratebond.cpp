/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007, 2008, 2010 Ferdinando Ametrano
 Copyright (C) 2009 Piter Dias

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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
                                 Schedule schedule,
                                 const std::vector<Rate>& coupons,
                                 const DayCounter& accrualDayCounter,
                                 BusinessDayConvention paymentConvention,
                                 Real redemption,
                                 const Date& issueDate,
                                 const Calendar& paymentCalendar,
                                 const Period& exCouponPeriod,
                                 const Calendar& exCouponCalendar,
                                 const BusinessDayConvention exCouponConvention,
                                 bool exCouponEndOfMonth,
                                 const DayCounter& firstPeriodDayCounter)
     : Bond(settlementDays,
            paymentCalendar==Calendar() ? schedule.calendar() : paymentCalendar,
            issueDate),
       frequency_(schedule.hasTenor() ? schedule.tenor().frequency() : NoFrequency),
       dayCounter_(accrualDayCounter),
       firstPeriodDayCounter_(firstPeriodDayCounter) {

        maturityDate_ = schedule.endDate();

        cashflows_ = FixedRateLeg(std::move(schedule))
            .withNotionals(faceAmount)
            .withCouponRates(coupons, accrualDayCounter)
            .withFirstPeriodDayCounter(firstPeriodDayCounter)
            .withPaymentCalendar(calendar_)
            .withPaymentAdjustment(paymentConvention)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
        QL_ENSURE(redemptions_.size() == 1, "multiple redemptions created");
    }

}

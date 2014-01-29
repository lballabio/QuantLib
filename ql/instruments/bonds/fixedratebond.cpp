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
                                 const Date& issueDate,
                                 const Calendar& paymentCalendar,
                                 const Period& exCouponPeriod,
                                 const Calendar& exCouponCalendar,
                                 const BusinessDayConvention exCouponConvention,
                                 bool exCouponEndOfMonth)
     : Bond(settlementDays,
            paymentCalendar==Calendar() ? schedule.calendar() : paymentCalendar,
            issueDate),
      frequency_(schedule.tenor().frequency()),
      dayCounter_(accrualDayCounter) {

        maturityDate_ = schedule.endDate();

        cashflows_ = FixedRateLeg(schedule)
            .withNotionals(faceAmount)
            .withCouponRates(coupons, accrualDayCounter)
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
                                 DateGeneration::Rule rule,
                                 bool endOfMonth,
                                 const Calendar& paymentCalendar,
                                 const Period& exCouponPeriod,
                                 const Calendar& exCouponCalendar,
                                 const BusinessDayConvention exCouponConvention,
                                 bool exCouponEndOfMonth)
     : Bond(settlementDays,
            paymentCalendar==Calendar() ? calendar : paymentCalendar,
            issueDate),
      frequency_(tenor.frequency()), dayCounter_(accrualDayCounter) {

        maturityDate_ = maturityDate;

        Date firstDate, nextToLastDate;
        switch (rule) {
          case DateGeneration::Backward:
            firstDate = Date();
            nextToLastDate = stubDate;
            break;
          case DateGeneration::Forward:
            firstDate = stubDate;
            nextToLastDate = Date();
            break;
          case DateGeneration::Zero:
          case DateGeneration::ThirdWednesday:
          case DateGeneration::Twentieth:
          case DateGeneration::TwentiethIMM:
            QL_FAIL("stub date (" << stubDate << ") not allowed with " <<
                    rule << " DateGeneration::Rule");
          default:
            QL_FAIL("unknown DateGeneration::Rule (" << Integer(rule) << ")");
        }

        Schedule schedule(startDate, maturityDate_, tenor,
                          calendar, accrualConvention, accrualConvention,
                          rule, endOfMonth,
                          firstDate, nextToLastDate);

        cashflows_ = FixedRateLeg(schedule)
            .withNotionals(faceAmount)
            .withCouponRates(coupons, accrualDayCounter)
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

    FixedRateBond::FixedRateBond(Natural settlementDays,
                                 Real faceAmount,
                                 const Schedule& schedule,
                                 const std::vector<InterestRate>& coupons,
                                 BusinessDayConvention paymentConvention,
                                 Real redemption,
                                 const Date& issueDate,
                                 const Calendar& paymentCalendar,
                                 const Period& exCouponPeriod,
                                 const Calendar& exCouponCalendar,
                                 const BusinessDayConvention exCouponConvention,
                                 bool exCouponEndOfMonth)
     : Bond(settlementDays,
            paymentCalendar==Calendar() ? schedule.calendar() : paymentCalendar,
            issueDate),
      frequency_(schedule.tenor().frequency()),
      dayCounter_(coupons[0].dayCounter()) {

        maturityDate_ = schedule.endDate();

        cashflows_ = FixedRateLeg(schedule)
            .withNotionals(faceAmount)
            .withCouponRates(coupons)
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

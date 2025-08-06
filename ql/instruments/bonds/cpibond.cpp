/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Chris Kenyon

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/instruments/bonds/cpibond.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>


namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    CPIBond::CPIBond(Natural settlementDays,
                     Real faceAmount,
                     Real baseCPI,
                     const Period& observationLag,
                     ext::shared_ptr<ZeroInflationIndex> cpiIndex,
                     CPI::InterpolationType observationInterpolation,
                     Schedule schedule,
                     const std::vector<Rate>& fixedRate,
                     const DayCounter& accrualDayCounter,
                     BusinessDayConvention paymentConvention,
                     const Date& issueDate,
                     const Calendar& paymentCalendar,
                     const Period& exCouponPeriod,
                     const Calendar& exCouponCalendar,
                     const BusinessDayConvention exCouponConvention,
                     bool exCouponEndOfMonth)
    : CPIBond(settlementDays, faceAmount, false, baseCPI, observationLag, cpiIndex,
              observationInterpolation, schedule, fixedRate, accrualDayCounter,
              paymentConvention, issueDate, paymentCalendar, exCouponPeriod,
              exCouponCalendar, exCouponConvention, exCouponEndOfMonth) {}

    CPIBond::CPIBond(Natural settlementDays,
                     Real faceAmount,
                     bool growthOnly,
                     Real baseCPI,
                     const Period& observationLag,
                     ext::shared_ptr<ZeroInflationIndex> cpiIndex,
                     CPI::InterpolationType observationInterpolation,
                     Schedule schedule,
                     const std::vector<Rate>& fixedRate,
                     const DayCounter& accrualDayCounter,
                     BusinessDayConvention paymentConvention,
                     const Date& issueDate,
                     const Calendar& paymentCalendar,
                     const Period& exCouponPeriod,
                     const Calendar& exCouponCalendar,
                     const BusinessDayConvention exCouponConvention,
                     bool exCouponEndOfMonth)
    : Bond(settlementDays,
           paymentCalendar == Calendar() ? schedule.calendar() : paymentCalendar,
           issueDate),
      frequency_(schedule.tenor().frequency()), dayCounter_(accrualDayCounter),
      growthOnly_(growthOnly), baseCPI_(baseCPI), observationLag_(observationLag),
      cpiIndex_(std::move(cpiIndex)), observationInterpolation_(observationInterpolation) {

        maturityDate_ = schedule.endDate();

        cashflows_ = CPILeg(std::move(schedule), cpiIndex_,
                            baseCPI_, observationLag_)
            .withNotionals(faceAmount)
            .withFixedRates(fixedRate)
            .withPaymentDayCounter(accrualDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withPaymentCalendar(calendar_)
            .withObservationInterpolation(observationInterpolation_)
            .withSubtractInflationNominal(growthOnly_)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth);


        calculateNotionalsFromCashflows();

        redemptions_.push_back(cashflows_.back());

        registerWith(cpiIndex_);
        Leg::const_iterator i;
        for (i = cashflows_.begin(); i < cashflows_.end(); ++i) {
            registerWith(*i);
        }
    }

    QL_DEPRECATED_ENABLE_WARNING

}

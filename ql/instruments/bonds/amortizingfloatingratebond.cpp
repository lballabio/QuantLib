/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

#include <ql/instruments/bonds/amortizingfloatingratebond.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    AmortizingFloatingRateBond::AmortizingFloatingRateBond(
                                    Natural settlementDays,
                                    const std::vector<Real>& notionals,
                                    const Schedule& schedule,
                                    const ext::shared_ptr<IborIndex>& index,
                                    const DayCounter& paymentDayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Natural fixingDays,
                                    const std::vector<Real>& gearings,
                                    const std::vector<Spread>& spreads,
                                    const std::vector<Rate>& caps,
                                    const std::vector<Rate>& floors,
                                    bool inArrears,
                                    const Date& issueDate,
                                    const Period& exCouponPeriod,
                                    const Calendar& exCouponCalendar,
                                    const BusinessDayConvention exCouponConvention,
                                    bool exCouponEndOfMonth)
    : Bond(settlementDays, schedule.calendar(), issueDate) {

        maturityDate_ = schedule.endDate();

        cashflows_ = IborLeg(schedule, index)
            .withNotionals(notionals)
            .withPaymentDayCounter(paymentDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withFixingDays(fixingDays)
            .withGearings(gearings)
            .withSpreads(spreads)
            .withCaps(caps)
            .withFloors(floors)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth)
            .inArrears(inArrears);

        addRedemptionsToCashflows();

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");

        registerWith(index);
    }


   AmortizingFloatingRateBond::AmortizingFloatingRateBond(
                                    Natural settlementDays,
                                    const std::vector<Real>& notionals,
                                    const std::vector<Real>& redemptions,
                                    const Schedule& schedule,
                                    const ext::shared_ptr<IborIndex>& index,
                                    const DayCounter& paymentDayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Natural fixingDays, Natural paymentLag,
                                    const std::vector<Real>& gearings,
                                    const std::vector<Spread>& spreads,
                                    const std::vector<Rate>& caps,
                                    const std::vector<Rate>& floors,
                                    bool inArrears,
                                    const Date& issueDate,
                                    const Period& exCouponPeriod,
                                    const Calendar& exCouponCalendar,
                                    const BusinessDayConvention exCouponConvention,
                                    bool exCouponEndOfMonth)
    : Bond(settlementDays, schedule.calendar(), issueDate) {

        maturityDate_ = schedule.endDate();

        cashflows_ = IborLeg(schedule, index)
            .withNotionals(notionals)
            .withPaymentDayCounter(paymentDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withFixingDays(fixingDays)
            .withPaymentLag(paymentLag)
            .withGearings(gearings)
            .withSpreads(spreads)
            .withCaps(caps)
            .withFloors(floors)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth)
            .inArrears(inArrears);

        addRedemptionsToCashflows(redemptions);

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");

        registerWith(index);
    }

}

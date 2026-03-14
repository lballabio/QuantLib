/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

#include <ql/instruments/bonds/amortizingcmsratebond.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    AmortizingCmsRateBond::AmortizingCmsRateBond(
                                    Natural settlementDays,
                                    const std::vector<Real>& notionals,
                                    Schedule schedule,
                                    const ext::shared_ptr<SwapIndex>& index,
                                    const DayCounter& paymentDayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Natural fixingDays,
                                    const std::vector<Real>& gearings,
                                    const std::vector<Spread>& spreads,
                                    const std::vector<Rate>& caps,
                                    const std::vector<Rate>& floors,
                                    bool inArrears,
                                    const Date& issueDate,
                                    const std::vector<Real>& redemptions)
    : Bond(settlementDays, schedule.calendar(), issueDate) {

        maturityDate_ = schedule.endDate();

        cashflows_ = CmsLeg(std::move(schedule), index)
            .withNotionals(notionals)
            .withPaymentDayCounter(paymentDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withFixingDays(fixingDays)
            .withGearings(gearings)
            .withSpreads(spreads)
            .withCaps(caps)
            .withFloors(floors)
            .inArrears(inArrears);

        addRedemptionsToCashflows(redemptions);

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");

        registerWith(index);
    }

}

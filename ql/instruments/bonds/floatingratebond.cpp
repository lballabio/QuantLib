/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola

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

#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    FloatingRateBond::FloatingRateBond(
                           Natural settlementDays,
                           Real faceAmount,
                           const Schedule& schedule,
                           const boost::shared_ptr<IborIndex>& index,
                           const DayCounter& paymentDayCounter,
                           BusinessDayConvention paymentConvention,
                           Natural fixingDays,
                           const std::vector<Real>& gearings,
                           const std::vector<Spread>& spreads,
                           const std::vector<Rate>& caps,
                           const std::vector<Rate>& floors,
                           bool inArrears,
                           Real redemption,
                           const Date& issueDate)
    : Bond(settlementDays, schedule.calendar(), faceAmount) {

        firstAccrualDate_ = schedule.startDate();
        maturityDate_     = schedule.endDate();

        issueDate_ = (issueDate==Date() ? firstAccrualDate_ : issueDate);

        cashflows_ = IborLeg(std::vector<Real>(1, faceAmount_),
                             schedule,
                             index,
                             paymentDayCounter,
                             paymentConvention,
                             std::vector<Natural>(1,fixingDays),
                             gearings, spreads,
                             caps, floors,
                             inArrears);

        Date redemptionDate = calendar_.adjust(maturityDate_,
                                               paymentConvention);
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(faceAmount_*redemption/100.0, redemptionDate)));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");

        registerWith(index);
    }

    FloatingRateBond::FloatingRateBond(
                           Natural settlementDays,
                           Real faceAmount,
                           const Date& startDate,
                           const Date& maturityDate,
                           Frequency couponFrequency,
                           const Calendar& calendar,
                           const boost::shared_ptr<IborIndex>& index,
                           const DayCounter& accrualDayCounter,
                           BusinessDayConvention accrualConvention,
                           BusinessDayConvention paymentConvention,
                           Natural fixingDays,
                           const std::vector<Real>& gearings,
                           const std::vector<Spread>& spreads,
                           const std::vector<Rate>& caps,
                           const std::vector<Rate>& floors,
                           bool inArrears,
                           Real redemption,
                           const Date& issueDate,
                           const Date& stubDate,
                           bool fromEnd)
    : Bond(settlementDays, calendar, faceAmount) {

        firstAccrualDate_ = startDate;
        maturityDate_     = maturityDate;

        issueDate_ = (issueDate==Date() ? firstAccrualDate_ : issueDate);

        Date firstDate = (fromEnd ? Date() : stubDate);
        Date nextToLastDate = (fromEnd ? stubDate : Date());
        Schedule schedule(firstAccrualDate_, maturityDate_, Period(couponFrequency),
                          calendar_, accrualConvention, accrualConvention,
                          fromEnd, false, firstDate, nextToLastDate);

        cashflows_ = IborLeg(std::vector<Real>(1, faceAmount_),
                             schedule,
                             index,
                             accrualDayCounter,
                             paymentConvention,
                             std::vector<Natural>(1,fixingDays),
                             gearings, spreads,
                             caps, floors,
                             inArrears);

        Date redemptionDate = calendar_.adjust(maturityDate_,
                                               paymentConvention);
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(faceAmount_*redemption/100.0, redemptionDate)));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");

        registerWith(index);
    }

}

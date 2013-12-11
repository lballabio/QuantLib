/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

#include <ql/experimental/catbonds/catbond.hpp>
#include <ql/settings.hpp>
#include <ql/experimental/credit/loss.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/time/daycounters/actualactual.hpp>

using namespace std;

namespace QuantLib {

    void CatBond::arguments::validate() const {
        Bond::arguments::validate();
        QL_REQUIRE(notionalRisk, "null notionalRisk");
    }

    void CatBond::setupArguments(PricingEngine::arguments* args) const {

        CatBond::arguments* arguments =
            dynamic_cast<CatBond::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong arguments type");

        Bond::setupArguments(args);

        arguments->notionalRisk = notionalRisk_;
        arguments->startDate = issueDate();
    }
    
    void CatBond::fetchResults(const PricingEngine::results* r) const {
        Bond::fetchResults(r);

        const CatBond::results* results =
            dynamic_cast<const CatBond::results*>(r);
        QL_ENSURE(results != 0, "wrong result type");

        lossProbability_ = results->lossProbability;
        expectedLoss_ = results->expectedLoss;
        exhaustionProbability_ = results->exhaustionProbability;
    }

    FloatingCatBond::FloatingCatBond(
                           Natural settlementDays,
                           Real faceAmount,
                           const Schedule& schedule,
                           const boost::shared_ptr<IborIndex>& iborIndex,
                           const DayCounter& paymentDayCounter,
                           boost::shared_ptr<NotionalRisk> notionalRisk,
                           BusinessDayConvention paymentConvention,
                           Natural fixingDays,
                           const std::vector<Real>& gearings,
                           const std::vector<Spread>& spreads,
                           const std::vector<Rate>& caps,
                           const std::vector<Rate>& floors,
                           bool inArrears,
                           Real redemption,
                           const Date& issueDate)
        : CatBond(settlementDays, schedule.calendar(), issueDate, notionalRisk) {

        maturityDate_ = schedule.endDate();

        cashflows_ = IborLeg(schedule, iborIndex)
            .withNotionals(faceAmount)
            .withPaymentDayCounter(paymentDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withFixingDays(fixingDays)
            .withGearings(gearings)
            .withSpreads(spreads)
            .withCaps(caps)
            .withFloors(floors)
            .inArrears(inArrears);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
        QL_ENSURE(redemptions_.size() == 1, "multiple redemptions created");

        registerWith(iborIndex);
    }

    FloatingCatBond::FloatingCatBond(
                           Natural settlementDays,
                           Real faceAmount,
                           const Date& startDate,
                           const Date& maturityDate,
                           Frequency couponFrequency,
                           const Calendar& calendar,
                           const boost::shared_ptr<IborIndex>& iborIndex,
                           const DayCounter& accrualDayCounter,
                           boost::shared_ptr<NotionalRisk> notionalRisk,
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
                           DateGeneration::Rule rule,
                           bool endOfMonth)
    : CatBond(settlementDays, calendar, issueDate, notionalRisk) {

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

        Schedule schedule(startDate, maturityDate_, Period(couponFrequency),
                          calendar_, accrualConvention, accrualConvention,
                          rule, endOfMonth,
                          firstDate, nextToLastDate);

        cashflows_ = IborLeg(schedule, iborIndex)
            .withNotionals(faceAmount)
            .withPaymentDayCounter(accrualDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withFixingDays(fixingDays)
            .withGearings(gearings)
            .withSpreads(spreads)
            .withCaps(caps)
            .withFloors(floors)
            .inArrears(inArrears);

        addRedemptionsToCashflows(std::vector<Real>(1, redemption));

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
        QL_ENSURE(redemptions_.size() == 1, "multiple redemptions created");

        registerWith(iborIndex);
    }

}

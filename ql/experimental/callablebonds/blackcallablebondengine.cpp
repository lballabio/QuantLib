/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

#include <ql/experimental/callablebonds/blackcallablebondengine.hpp>
#include <ql/experimental/callablebonds/callablebondvolstructure.hpp>
#include <ql/experimental/callablebonds/callablebondconstantvol.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

using namespace std;

namespace QuantLib {

    BlackCallableFixedRateBondEngine::BlackCallableFixedRateBondEngine(
                              const Handle<Quote>& fwdYieldVol,
                              const Handle<YieldTermStructure>& discountCurve)
    : volatility_(boost::shared_ptr<CallableBondVolatilityStructure>(
                      new CallableBondConstantVolatility(0, NullCalendar(),
                                                         fwdYieldVol,
                                                         Actual365Fixed()))),
      discountCurve_(discountCurve) {
        registerWith(volatility_);
        registerWith(discountCurve_);
    }

    //! no vol structures implemented yet besides constant volatility
    BlackCallableFixedRateBondEngine::BlackCallableFixedRateBondEngine(
             const Handle<CallableBondVolatilityStructure>& yieldVolStructure,
             const Handle<YieldTermStructure>& discountCurve)
    : volatility_(yieldVolStructure), discountCurve_(discountCurve) {
        registerWith(volatility_);
        registerWith(discountCurve_);
    }

    Real BlackCallableFixedRateBondEngine::spotIncome() const {
        //! settle date of embedded option assumed same as that of bond
        Date settlement = arguments_.settlementDate;
        Leg cf = arguments_.cashflows;
        Date optionMaturity = arguments_.putCallSchedule[0]->date();

        /* the following assumes
           1. cashflows are in ascending order !
           2. income = coupons paid between settlementDate() and put/call date
        */
        Real income = 0.0;
        for (Size i = 0; i < cf.size() - 1; ++i) {
            if (!cf[i]->hasOccurred(settlement, false)) {
                if (cf[i]->hasOccurred(optionMaturity, false)) {
                    income += cf[i]->amount() *
                              discountCurve_->discount(cf[i]->date());
                } else {
                    break;
                }
            }
        }
        return income/discountCurve_->discount(settlement);
    }


    Volatility BlackCallableFixedRateBondEngine::forwardPriceVolatility()
                                                                       const {
        Date bondMaturity = arguments_.redemptionDate;
        Date exerciseDate = arguments_.callabilityDates[0];
        Leg fixedLeg = arguments_.cashflows;

        // value of bond cash flows at option maturity
        Real fwdNpv = CashFlows::npv(fixedLeg,
                                     **discountCurve_,
                                     false, exerciseDate);

        DayCounter dayCounter = arguments_.paymentDayCounter;
        Frequency frequency = arguments_.frequency;

        // adjust if zero coupon bond (see also bond.cpp)
        if (frequency == NoFrequency || frequency == Once)
            frequency = Annual;

        Rate fwdYtm = CashFlows::yield(fixedLeg,
                                       fwdNpv,
                                       dayCounter,
                                       Compounded,
                                       frequency,
                                       false, exerciseDate);

        InterestRate fwdRate(fwdYtm,
                             dayCounter,
                             Compounded,
                             frequency);

        Time fwdDur = CashFlows::duration(fixedLeg,
                                          fwdRate,
                                          Duration::Modified,
                                          false, exerciseDate);

        Real cashStrike = arguments_.callabilityPrices[0];
        dayCounter = volatility_->dayCounter();
        Date referenceDate = volatility_->referenceDate();
        Time exerciseTime = dayCounter.yearFraction(referenceDate,
                                                    exerciseDate);
        Time maturityTime = dayCounter.yearFraction(referenceDate,
                                                    bondMaturity);
        Volatility yieldVol = volatility_->volatility(exerciseTime,
                                                      maturityTime-exerciseTime,
                                                      cashStrike);
        Volatility fwdPriceVol = yieldVol*fwdDur*fwdYtm;
        return fwdPriceVol;
    }


    void BlackCallableFixedRateBondEngine::calculate() const {
        // validate args for Black engine
        QL_REQUIRE(arguments_.putCallSchedule.size() == 1,
                   "Must have exactly one call/put date to use Black Engine");

        Date settle = arguments_.settlementDate;
        Date exerciseDate = arguments_.callabilityDates[0];
        QL_REQUIRE(exerciseDate >= settle,
                   "must have exercise Date >= settlement Date");

        Leg fixedLeg = arguments_.cashflows;

        Real value = CashFlows::npv(fixedLeg,
                                    **discountCurve_,
                                    false, settle);

        Real npv = CashFlows::npv(fixedLeg,
                                  **discountCurve_,
                                  false, discountCurve_->referenceDate());

        Real fwdCashPrice = (value - spotIncome())/
                            discountCurve_->discount(exerciseDate);

        Real cashStrike = arguments_.callabilityPrices[0];

        Option::Type type = (arguments_.putCallSchedule[0]->type() ==
                             Callability::Call ? Option::Call : Option::Put);

        Volatility priceVol = forwardPriceVolatility();

        Time exerciseTime = volatility_->dayCounter().yearFraction(
                                                 volatility_->referenceDate(),
                                                 exerciseDate);
        Real embeddedOptionValue =
            blackFormula(type,
                         cashStrike,
                         fwdCashPrice,
                         priceVol*std::sqrt(exerciseTime));

        if (type == Option::Call) {
            results_.value = npv - embeddedOptionValue;
            results_.settlementValue = value - embeddedOptionValue;
        } else {
            results_.value = npv + embeddedOptionValue;
            results_.settlementValue = value + embeddedOptionValue;
        }
    }

}

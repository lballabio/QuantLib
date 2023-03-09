/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2023 Marcin Rybacki

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

#include <ql/instruments/equitytotalreturnswap.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/equitycashflow.hpp>

namespace QuantLib {

    namespace {
        ext::shared_ptr<CashFlow>
        createEquityCashFlow(const Schedule& schedule,
                             const ext::shared_ptr<EquityIndex>& equityIndex,
                             Real nominal,
                             const Calendar& paymentCalendar,
                             BusinessDayConvention paymentConvention,
                             Natural paymentDelay) {
            Date startDate = schedule.startDate();
            Date endDate = schedule.endDate();

            Calendar cal = paymentCalendar;
            if (cal.empty()) {
                QL_REQUIRE(!schedule.calendar().empty(), "Calendar in schedule cannot be empty");
                cal = schedule.calendar();
            }
            Date paymentDate =
                cal.advance(endDate, paymentDelay, Days, paymentConvention, schedule.endOfMonth());
            return ext::make_shared<EquityCashFlow>(nominal, equityIndex, startDate, endDate,
                                                    paymentDate);
        }

        template <typename IndexType, typename LegType>
        Leg createInterestLeg(const Schedule& schedule,
                              const ext::shared_ptr<IndexType>& interestRateIndex,
                              Real nominal,
                              const DayCounter& dayCounter,
                              Rate margin,
                              Real gearing,
                              const Calendar& paymentCalendar,
                              BusinessDayConvention paymentConvention,
                              Natural paymentDelay) {
            return LegType(schedule, interestRateIndex)
                .withNotionals(nominal)
                .withPaymentDayCounter(dayCounter)
                .withSpreads(margin)
                .withGearings(gearing)
                .withPaymentCalendar(paymentCalendar)
                .withPaymentAdjustment(paymentConvention)
                .withPaymentLag(paymentDelay);
        }
    }

    EquityTotalReturnSwap::EquityTotalReturnSwap(ext::shared_ptr<EquityIndex> equityIndex,
                                                 const ext::shared_ptr<InterestRateIndex>& interestRateIndex,
                                                 Type type,
                                                 Real nominal,
                                                 Schedule schedule,
                                                 DayCounter dayCounter,
                                                 Rate margin,
                                                 Real gearing,
                                                 Calendar paymentCalendar,
                                                 BusinessDayConvention paymentConvention,
                                                 Natural paymentDelay)
    : Swap(2), equityIndex_(std::move(equityIndex)), interestRateIndex_(interestRateIndex),
      type_(type), nominal_(nominal), schedule_(std::move(schedule)),
      dayCounter_(std::move(dayCounter)), margin_(margin), gearing_(gearing),
      paymentCalendar_(std::move(paymentCalendar)), paymentConvention_(paymentConvention),
      paymentDelay_(paymentDelay) {

        QL_REQUIRE(!(nominal_ < 0.0), "Nominal cannot be negative");

        legs_[0].push_back(createEquityCashFlow(schedule_, equityIndex_, nominal_, paymentCalendar_,
                                                paymentConvention_, paymentDelay_));
        for (Leg::const_iterator i = legs_[0].begin(); i < legs_[0].end(); ++i)
            registerWith(*i);

        switch (type_) {
            case Payer:
                payer_[0] = -1.0;
                payer_[1] = +1.0;
                break;
            case Receiver:
                payer_[0] = +1.0;
                payer_[1] = -1.0;
                break;
            default:
                QL_FAIL("unknown equity total return swap type");
        }
    }

    EquityTotalReturnSwap::EquityTotalReturnSwap(Type type,
                                                 Real nominal,
                                                 Schedule schedule,
                                                 ext::shared_ptr<EquityIndex> equityIndex,
                                                 const ext::shared_ptr<IborIndex>& interestRateIndex,
                                                 DayCounter dayCounter,
                                                 Rate margin,
                                                 Real gearing,
                                                 Calendar paymentCalendar,
                                                 BusinessDayConvention paymentConvention,
                                                 Natural paymentDelay)
    : EquityTotalReturnSwap(std::move(equityIndex),
                            interestRateIndex,
                            type, 
                            nominal, 
                            std::move(schedule), 
                            std::move(dayCounter),
                            margin,
                            gearing,
                            std::move(paymentCalendar),
                            paymentConvention, 
                            paymentDelay) {
        legs_[1] = createInterestLeg<IborIndex, IborLeg>(
            schedule_, interestRateIndex, nominal_, dayCounter_, margin_, gearing_,
            paymentCalendar_, paymentConvention_, paymentDelay_);
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);
    }

    EquityTotalReturnSwap::EquityTotalReturnSwap(Type type,
                                                 Real nominal,
                                                 Schedule schedule,
                                                 ext::shared_ptr<EquityIndex> equityIndex,
                                                 const ext::shared_ptr<OvernightIndex>& interestRateIndex,
                                                 DayCounter dayCounter,
                                                 Rate margin,
                                                 Real gearing,
                                                 Calendar paymentCalendar,
                                                 BusinessDayConvention paymentConvention,
                                                 Natural paymentDelay)
    : EquityTotalReturnSwap(std::move(equityIndex),
                            interestRateIndex,
                            type, 
                            nominal, 
                            std::move(schedule), 
                            std::move(dayCounter),
                            margin,
                            gearing,
                            std::move(paymentCalendar),
                            paymentConvention, 
                            paymentDelay) {
        legs_[1] = createInterestLeg<OvernightIndex, OvernightLeg>(
            schedule_, interestRateIndex, nominal_, dayCounter_, margin_, gearing_,
            paymentCalendar_, paymentConvention_, paymentDelay_);
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);
    }

    const Leg& EquityTotalReturnSwap::equityLeg() const {
        return leg(0);
    }

    const Leg& EquityTotalReturnSwap::interestRateLeg() const {
        return leg(1);
    }

    Real EquityTotalReturnSwap::equityLegNPV() const {
        return legNPV(0);
    }

    Real EquityTotalReturnSwap::interestRateLegNPV() const {
        return legNPV(1);
    }

    Real EquityTotalReturnSwap::fairMargin() const {
        // Knowing that for the fair margin NPV = 0.0, where:
        // NPV = NPV Equity Leg + [NPV Floating Leg + margin * BPS / 10000]
        // hence,
        // fair margin = - [NPV Equity Leg + NPV Floating Leg] / BPS * 10000
        const Spread basisPoint = 1.0e-4;
        Real interestLegBps = legBPS(1) / basisPoint;
        Real exMarginInterestLegNpv = interestRateLegNPV() - margin() * interestLegBps;
        return -(equityLegNPV() + exMarginInterestLegNpv) / interestLegBps;
    }
}
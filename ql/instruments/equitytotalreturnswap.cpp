/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2023 Marcin Rybacki

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

#include <ql/cashflows/equitycashflow.hpp>
#include <ql/cashflows/equitynotionalresetcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/instruments/equitytotalreturnswap.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    namespace {
        Leg createEquityLeg(const Schedule& schedule,
                            const ext::shared_ptr<EquityIndex>& equityIndex,
                            Real nominal,
                            Frequency equityPaymentFrequency,
                            bool notionalReset,
                            const Calendar& paymentCalendar,
                            BusinessDayConvention paymentConvention,
                            Natural paymentDelay) {
            QL_REQUIRE(schedule.size() >= 2, "Schedule must contain at least two dates");
            QL_REQUIRE(!notionalReset || equityPaymentFrequency != Once,
                       "Notional reset requires periodic equity payments");

            Calendar cal = paymentCalendar;
            if (cal.empty()) {
                QL_REQUIRE(!schedule.calendar().empty(), "Calendar in schedule cannot be empty");
                cal = schedule.calendar();
            }

            const bool endOfMonth = schedule.hasEndOfMonth() && schedule.endOfMonth();
            Leg leg;
            const auto addCashFlow = [&](const Date& startDate, const Date& endDate) {
                Date paymentDate = cal.advance(endDate, paymentDelay, Days, paymentConvention,
                                               endOfMonth);
                if (notionalReset)
                    leg.push_back(ext::make_shared<EquityNotionalResetCashFlow>(
                        nominal, equityIndex, schedule.startDate(), startDate, endDate,
                        paymentDate));
                else
                    leg.push_back(ext::make_shared<EquityCashFlow>(
                        nominal, equityIndex, startDate, endDate, paymentDate));
            };

            if (equityPaymentFrequency == Once) {
                addCashFlow(schedule.startDate(), schedule.endDate());
            } else {
                QL_REQUIRE(equityPaymentFrequency != NoFrequency &&
                               equityPaymentFrequency != OtherFrequency,
                           "unsupported equity payment frequency: " << equityPaymentFrequency);
                const Period tenor(equityPaymentFrequency);
                DateGeneration::Rule rule = DateGeneration::Backward;
                if (schedule.hasRule() && (schedule.rule() == DateGeneration::Forward ||
                                           schedule.rule() == DateGeneration::Backward))
                    rule = schedule.rule();
                const BusinessDayConvention terminationConvention =
                    schedule.hasTerminationDateBusinessDayConvention()
                        ? schedule.terminationDateBusinessDayConvention()
                        : schedule.businessDayConvention();
                const Schedule equitySchedule(
                    schedule.startDate(), schedule.endDate(), tenor, schedule.calendar(),
                    schedule.businessDayConvention(), terminationConvention, rule, endOfMonth);
                if (notionalReset) {
                    // the notional of each funding period is reset to the index
                    // level at the start of the corresponding equity period
                    QL_REQUIRE(equitySchedule.size() == schedule.size(),
                               "Notional reset requires equity periods matching the "
                               "funding periods; the equity payment frequency ("
                                   << equityPaymentFrequency << ") gives "
                                   << equitySchedule.size() - 1 << " equity periods against "
                                   << schedule.size() - 1 << " funding periods");
                    for (Size i = 0; i < schedule.size(); ++i)
                        QL_REQUIRE(equitySchedule[i] == schedule[i],
                                   "Notional reset requires equity periods matching the "
                                   "funding periods; equity date "
                                       << equitySchedule[i] << " differs from funding date "
                                       << schedule[i]);
                }
                leg.reserve(equitySchedule.size() - 1);
                for (Size i = 1; i < equitySchedule.size(); ++i) {
                    QL_REQUIRE(equitySchedule[i] > equitySchedule[i - 1],
                               "Equity schedule dates must be strictly increasing");
                    addCashFlow(equitySchedule[i - 1], equitySchedule[i]);
                }
            }
            return leg;
        }

        template <typename IndexType, typename LegType>
        Leg createInterestLeg(const Schedule& schedule,
                              const ext::shared_ptr<IndexType>& interestRateIndex,
                              const Leg& equityLeg,
                              Real nominal,
                              bool notionalReset,
                              const DayCounter& dayCounter,
                              Rate margin,
                              Real gearing,
                              const Calendar& paymentCalendar,
                              BusinessDayConvention paymentConvention,
                              Natural paymentDelay) {
            Leg leg = LegType(schedule, interestRateIndex)
                .withNotionals(nominal)
                .withPaymentDayCounter(dayCounter)
                .withSpreads(margin)
                .withGearings(gearing)
                .withPaymentCalendar(paymentCalendar)
                .withPaymentAdjustment(paymentConvention)
                .withPaymentLag(paymentDelay);
            if (notionalReset) {
                // each funding coupon accrues on the notional of the equity
                // cash flow of the same period, including any adjustment made
                // by the pricer of the latter (e.g., a quanto adjustment)
                QL_REQUIRE(leg.size() == equityLeg.size(),
                           "Notional reset requires as many funding coupons ("
                               << leg.size() << ") as equity cash flows (" << equityLeg.size()
                               << ")");
                for (Size i = 0; i < leg.size(); ++i) {
                    auto coupon = ext::dynamic_pointer_cast<FloatingRateCoupon>(leg[i]);
                    QL_REQUIRE(coupon, "unexpected non-coupon cash flow on the funding leg");
                    auto equityCashFlow =
                        ext::dynamic_pointer_cast<EquityNotionalResetCashFlow>(equityLeg[i]);
                    QL_REQUIRE(equityCashFlow, "unexpected cash flow on the equity leg");
                    QL_REQUIRE(equityCashFlow->baseDate() == coupon->accrualStartDate(),
                               "Notional reset requires equity periods matching the funding "
                               "periods; equity period starting on "
                                   << equityCashFlow->baseDate()
                                   << " against funding period starting on "
                                   << coupon->accrualStartDate());
                    leg[i] = ext::make_shared<EquityNotionalResetCoupon>(coupon, equityCashFlow);
                }
            }
            return leg;
        }

        Leg createDividendLeg(const DividendSchedule& dividends,
                              const Leg& equityLeg,
                              Real dividendFactor) {
            DividendSchedule sorted(dividends);
            for (const auto& d : sorted)
                QL_REQUIRE(d, "null dividend given");
            std::stable_sort(sorted.begin(), sorted.end(),
                             [](const ext::shared_ptr<Dividend>& d1,
                                const ext::shared_ptr<Dividend>& d2) {
                                 return d1->date() < d2->date();
                             });

            Leg leg;
            leg.reserve(sorted.size());
            for (const auto& d : sorted) {
                const Date exDate = d->date();
                // the dividend is paid on the units held over the equity
                // period in which its ex-dividend date falls
                ext::shared_ptr<EquityCashFlow> period;
                for (const auto& cf : equityLeg) {
                    auto equityCashFlow = ext::dynamic_pointer_cast<EquityCashFlow>(cf);
                    QL_REQUIRE(equityCashFlow, "unexpected cash flow on the equity leg");
                    if (exDate > equityCashFlow->baseDate() &&
                        exDate <= equityCashFlow->fixingDate()) {
                        period = equityCashFlow;
                        break;
                    }
                }
                QL_REQUIRE(period, "ex-dividend date (" << exDate
                                                        << ") outside the equity periods");
                leg.push_back(ext::make_shared<EquityDividendCashFlow>(d, period, dividendFactor,
                                                                       period->date()));
            }
            return leg;
        }
    }

    EquityTotalReturnSwap::EquityTotalReturnSwap(
        ext::shared_ptr<EquityIndex> equityIndex,
        ext::shared_ptr<InterestRateIndex> interestRateIndex,
        Type type,
        Real nominal,
        Schedule schedule,
        DayCounter dayCounter,
        Rate margin,
        Real gearing,
        Calendar paymentCalendar,
        BusinessDayConvention paymentConvention,
        Natural paymentDelay,
        Frequency equityPaymentFrequency,
        bool notionalReset,
        DividendSchedule dividends,
        Real dividendFactor)
    : Swap(dividends.empty() ? 2 : 3), equityIndex_(std::move(equityIndex)),
      interestRateIndex_(std::move(interestRateIndex)), type_(type), nominal_(nominal),
      schedule_(std::move(schedule)), dayCounter_(std::move(dayCounter)), margin_(margin),
      gearing_(gearing), paymentCalendar_(std::move(paymentCalendar)),
      paymentConvention_(paymentConvention), paymentDelay_(paymentDelay),
      equityPaymentFrequency_(equityPaymentFrequency), notionalReset_(notionalReset),
      dividends_(std::move(dividends)), dividendFactor_(dividendFactor) {

        QL_REQUIRE(!(nominal_ < 0.0), "Nominal cannot be negative");
        QL_REQUIRE(!(dividendFactor_ < 0.0), "Dividend factor cannot be negative");

        legs_[0] = createEquityLeg(schedule_, equityIndex_, nominal_, equityPaymentFrequency_,
                                  notionalReset_, paymentCalendar_, paymentConvention_,
                                  paymentDelay_);
        for (auto i = legs_[0].begin(); i < legs_[0].end(); ++i)
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

        if (!dividends_.empty()) {
            QL_REQUIRE(equityIndex_, "no equity index given");
            // total and excess return indexes already reinvest the dividends,
            // which would be paid twice
            QL_REQUIRE(equityIndex_->returnType() == EquityIndex::Price,
                       "Dividend leg requires a price return equity index");
            legs_[2] = createDividendLeg(dividends_, legs_[0], dividendFactor_);
            for (auto i = legs_[2].begin(); i < legs_[2].end(); ++i)
                registerWith(*i);
            // dividends are received by the receiver of the equity leg
            payer_[2] = payer_[0];
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
                                                 Natural paymentDelay,
                                                 Frequency equityPaymentFrequency,
                                                 bool notionalReset,
                                                 DividendSchedule dividends,
                                                 Real dividendFactor)
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
                            paymentDelay,
                            equityPaymentFrequency,
                            notionalReset,
                            std::move(dividends),
                            dividendFactor) {
        legs_[1] = createInterestLeg<IborIndex, IborLeg>(
            schedule_, interestRateIndex, legs_[0], nominal_, notionalReset_, dayCounter_,
            margin_, gearing_, paymentCalendar_, paymentConvention_, paymentDelay_);
        for (auto i = legs_[1].begin(); i < legs_[1].end(); ++i)
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
                                                 Natural paymentDelay,
                                                 Frequency equityPaymentFrequency,
                                                 bool notionalReset,
                                                 DividendSchedule dividends,
                                                 Real dividendFactor)
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
                            paymentDelay,
                            equityPaymentFrequency,
                            notionalReset,
                            std::move(dividends),
                            dividendFactor) {
        legs_[1] = createInterestLeg<OvernightIndex, OvernightLeg>(
            schedule_, interestRateIndex, legs_[0], nominal_, notionalReset_, dayCounter_,
            margin_, gearing_, paymentCalendar_, paymentConvention_, paymentDelay_);
        for (auto i = legs_[1].begin(); i < legs_[1].end(); ++i)
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

    const Leg& EquityTotalReturnSwap::dividendLeg() const {
        static const Leg noDividends;
        return hasDividendLeg() ? leg(2) : noDividends;
    }

    Real EquityTotalReturnSwap::interestRateLegNPV() const {
        return legNPV(1);
    }

    Real EquityTotalReturnSwap::dividendLegNPV() const {
        return hasDividendLeg() ? legNPV(2) : Real(0.0);
    }

    Real EquityTotalReturnSwap::fairMargin() const {
        // Knowing that for the fair margin NPV = 0.0, where:
        // NPV = NPV Equity Leg + NPV Dividend Leg
        //       + [NPV Floating Leg + margin * BPS / 10000]
        // hence,
        // fair margin = - [NPV Equity Leg + NPV Dividend Leg + NPV Floating Leg] / BPS * 10000
        const Spread basisPoint = 1.0e-4;
        Real interestLegBps = legBPS(1) / basisPoint;
        Real exMarginInterestLegNpv = interestRateLegNPV() - margin() * interestLegBps;
        return -(equityLegNPV() + dividendLegNPV() + exMarginInterestLegNpv) / interestLegBps;
    }
}

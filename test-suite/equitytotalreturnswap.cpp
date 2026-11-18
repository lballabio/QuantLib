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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/equitytotalreturnswap.hpp>
#include <ql/cashflows/equitycashflow.hpp>
#include <ql/cashflows/equitynotionalresetcoupon.hpp>
#include <ql/currencies/america.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(EquityTotalReturnSwapTests)

struct CommonVars {

    Date today;
    Calendar calendar;
    DayCounter dayCount;

    ext::shared_ptr<EquityIndex> equityIndex;
    ext::shared_ptr<IborIndex> usdLibor;
    ext::shared_ptr<OvernightIndex> sofr;
    RelinkableHandle<YieldTermStructure> interestHandle;
    RelinkableHandle<YieldTermStructure> dividendHandle;
    ext::shared_ptr<Quote> spot;
    RelinkableHandle<Quote> spotHandle;
    ext::shared_ptr<PricingEngine> discountEngine;

    // utilities

    CommonVars() {
        calendar = UnitedStates(UnitedStates::GovernmentBond);
        dayCount = Actual365Fixed();

        today = calendar.adjust(Date(27, January, 2023));
        Settings::instance().evaluationDate() = today;

        equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, USDCurrency(), interestHandle,
                                                    dividendHandle, spotHandle);
        equityIndex->addFixing(Date(5, January, 2023), 9010.0);
        equityIndex->addFixing(today, 8690.0);

        sofr = ext::make_shared<Sofr>(interestHandle);
        sofr->addFixing(Date(3, January, 2023), 0.03);
        sofr->addFixing(Date(4, January, 2023), 0.031);
        sofr->addFixing(Date(5, January, 2023), 0.031);
        sofr->addFixing(Date(6, January, 2023), 0.031);
        sofr->addFixing(Date(9, January, 2023), 0.032);
        sofr->addFixing(Date(10, January, 2023), 0.033);
        sofr->addFixing(Date(11, January, 2023), 0.033);
        sofr->addFixing(Date(12, January, 2023), 0.033);
        sofr->addFixing(Date(13, January, 2023), 0.033);
        sofr->addFixing(Date(17, January, 2023), 0.033);
        sofr->addFixing(Date(18, January, 2023), 0.034);
        sofr->addFixing(Date(19, January, 2023), 0.034);
        sofr->addFixing(Date(20, January, 2023), 0.034);
        sofr->addFixing(Date(23, January, 2023), 0.034);
        sofr->addFixing(Date(24, January, 2023), 0.034);
        sofr->addFixing(Date(25, January, 2023), 0.034);
        sofr->addFixing(Date(26, January, 2023), 0.034);

        usdLibor = ext::make_shared<USDLibor>(3 * Months, interestHandle);
        usdLibor->addFixing(Date(3, January, 2023), 0.035);

        interestHandle.linkTo(flatRate(0.0375, dayCount));
        dividendHandle.linkTo(flatRate(0.005, dayCount));

        discountEngine =
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(interestHandle));

        spot = ext::make_shared<SimpleQuote>(8700.0);
        spotHandle.linkTo(spot);
    }

    ext::shared_ptr<EquityTotalReturnSwap> createTRSWithFrequency(
        Swap::Type type, const Schedule& schedule, bool useOvernightIndex,
        Frequency frequency, Rate margin = 0.0,
        Natural paymentDelay = 0, Calendar paymentCalendar = Calendar(),
        bool notionalReset = false) {
        if (paymentCalendar.empty())
            paymentCalendar = schedule.calendar();
        ext::shared_ptr<EquityTotalReturnSwap> swap;
        if (useOvernightIndex) {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                type, 100000.0, schedule, equityIndex, sofr, dayCount, margin,
                1.0, paymentCalendar, Following, paymentDelay, frequency, notionalReset);
        } else {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                type, 100000.0, schedule, equityIndex, usdLibor, dayCount, margin,
                1.0, paymentCalendar, Following, paymentDelay, frequency, notionalReset);
        }
        swap->setPricingEngine(discountEngine);
        return swap;
    }

    ext::shared_ptr<EquityTotalReturnSwap> createTRS(Swap::Type type,
                                                     const Schedule& schedule,
                                                     bool useOvernightIndex,
                                                     Rate margin = 0.0,
                                                     Real nominal = 1.0e7,
                                                     Real gearing = 1.0,
                                                     Natural paymentDelay = 0) {
        ext::shared_ptr<EquityTotalReturnSwap> swap;
        if (useOvernightIndex) {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                    type, nominal, schedule, equityIndex, sofr, dayCount, margin, gearing,
                    schedule.calendar(), Following, paymentDelay);
        } else {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                    type, nominal, schedule, equityIndex, usdLibor, dayCount, margin, gearing,
                    schedule.calendar(), Following, paymentDelay);
        }
        swap->setPricingEngine(discountEngine);
        return swap;
    }

    ext::shared_ptr<EquityTotalReturnSwap> createTRS(Swap::Type type,
                                                     const Date& start,
                                                     const Date& end,
                                                     bool useOvernightIndex,
                                                     Rate margin = 0.0,
                                                     Real nominal = 1.0e7,
                                                     Real gearing = 1.0,
                                                     Natural paymentDelay = 0) {
        Schedule schedule = MakeSchedule()
            .from(start)
            .to(end)
            .withTenor(3 * Months)
            .withCalendar(calendar)
            .withConvention(Following)
            .backwards();
        return createTRS(type, schedule, useOvernightIndex, margin, nominal, gearing,
                         paymentDelay);
    }
};

void checkFairMarginCalculation(Swap::Type type,
                                const Date& start,
                                const Date& end,
                                bool useOvernightIndex,
                                Rate margin = 0.0,
                                Real gearing = 1.0,
                                Natural paymentDelay = 0) {
    CommonVars vars;

    const Real tolerance = 1.0e-8;
    const Real nominal = 1.0e7;

    auto trs = vars.createTRS(type, start, end, useOvernightIndex, margin, nominal,
                              gearing, paymentDelay);
    auto fairMargin = trs->fairMargin();
    auto parTrs = vars.createTRS(type, start, end, useOvernightIndex, fairMargin,
                                 nominal, gearing, paymentDelay);

    if ((std::fabs(parTrs->NPV()) > tolerance))
        BOOST_ERROR("unable to imply a fair margin\n"
                    << "    actual NPV:    " << parTrs->NPV() << "\n"
                    << "    expected NPV:    0.0 \n"
                    << "    fair margin:    " << fairMargin << "\n"
                    << "    IR index name:    " << trs->interestRateIndex()->name() << "\n");
}

Real legNPV(const Leg& leg, const Handle<YieldTermStructure>& ts) {
    Real npv = 0.0;
    std::for_each(leg.begin(), leg.end(), [&](const ext::shared_ptr<CashFlow>& cf) {
        npv += cf->amount() * ts->discount(cf->date());
    });
    return npv;
}

void checkNPVCalculation(Swap::Type type,
                         const Date& start,
                         const Date& end,
                         bool useOvernightIndex,
                         Rate margin = 0.0,
                         Real gearing = 1.0,
                         Natural paymentDelay = 0) {
    CommonVars vars;

    const Real tolerance = 1.0e-2;
    const Real nominal = 1.0e7;

    auto trs = vars.createTRS(type, start, end, useOvernightIndex, margin, nominal,
                              gearing, paymentDelay);

    auto npv = trs->NPV();

    Real scaling = type == Swap::Type::Receiver ? 1.0 : -1.0;
    auto equityLegNPV = trs->equityLegNPV();
    auto replicatedEquityLegNPV = scaling * legNPV(trs->equityLeg(), vars.interestHandle);

    if ((std::fabs(equityLegNPV - replicatedEquityLegNPV) > tolerance))
        BOOST_ERROR("incorrect NPV of the equity leg\n"
                    << "    actual NPV:    " << equityLegNPV << "\n"
                    << "    expected NPV:    " << replicatedEquityLegNPV << "\n");

    auto interestLegNPV = trs->interestRateLegNPV();
    auto replicatedInterestLegNPV = -scaling * legNPV(trs->interestRateLeg(), vars.interestHandle);

    if ((std::fabs(interestLegNPV - replicatedInterestLegNPV) > tolerance))
        BOOST_ERROR("incorrect NPV of the interest leg\n"
                    << "    actual NPV:    " << interestLegNPV << "\n"
                    << "    expected NPV:    " << replicatedInterestLegNPV << "\n");

    if ((std::fabs(npv - (equityLegNPV + interestLegNPV)) > tolerance))
        BOOST_ERROR("summing legs NPV does not replicate the instrument NPV\n"
                    << "    actual NPV:    " << npv << "\n"
                    << "    NPV from summing legs:    " << equityLegNPV + interestLegNPV << "\n");
}


BOOST_AUTO_TEST_CASE(testPeriodicEquityCashFlows) {
    BOOST_TEST_MESSAGE("Testing periodic equity cash flows...");
    CommonVars vars;
    const std::vector<Date> dates = {Date(5, January, 2023), Date(5, April, 2023),
                                   Date(5, July, 2023), Date(5, October, 2023),
                                   Date(5, January, 2024)};
    const Schedule schedule(dates, vars.calendar, Unadjusted, Unadjusted,
                            3 * Months, DateGeneration::Forward, false);
    // Put every equity fixing in the past so amounts depend only on the supplied levels.
    Settings::instance().evaluationDate() = Date(8, January, 2024);

    // Historical levels exercise positive, negative and zero returns. Every payment
    // resets the base level, while the notional remains fixed at 100,000.
    const Real levels[] = {100.0, 110.0, 99.0, 118.8, 118.8};
    const std::vector<Real> quarterlyAmounts = {10000.0, -10000.0, 20000.0, 0.0};
    // Semiannual returns use 99/100 and 118.8/99, rather than adding quarterly returns.
    const std::vector<Real> semiannualAmounts = {-1000.0, 20000.0};
    for (Size i = 0; i < dates.size(); ++i)
        vars.equityIndex->addFixing(dates[i], levels[i], true);

    for (bool overnight : {false, true}) {
        for (Frequency frequency : {Quarterly, Semiannual}) {
            auto swap = vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, frequency);
            const auto& amounts = frequency == Quarterly ? quarterlyAmounts : semiannualAmounts;
            // A semiannual equity period spans two dates of the quarterly funding grid.
            const Size stride = frequency == Quarterly ? 1 : 2;
            BOOST_CHECK_EQUAL(swap->equityPaymentFrequency(), frequency);
            BOOST_REQUIRE_EQUAL(swap->equityLeg().size(), amounts.size());
            BOOST_REQUIRE_EQUAL(swap->interestRateLeg().size(), 4);
            for (Size i = 0; i < amounts.size(); ++i) {
                auto flow = ext::dynamic_pointer_cast<EquityCashFlow>(swap->equityLeg()[i]);
                BOOST_REQUIRE(flow);
                BOOST_CHECK_EQUAL(flow->baseDate(), dates[stride * i]);
                BOOST_CHECK_EQUAL(flow->fixingDate(), dates[stride * (i + 1)]);
                BOOST_CHECK_EQUAL(flow->date(), dates[stride * (i + 1)]);
                BOOST_CHECK_EQUAL(flow->notional(), 100000.0);
                // Growth-only settlement excludes repayment of the equity notional.
                // With no payment lag and business-day endpoints, fixing and payment coincide.
                BOOST_CHECK(flow->growthOnly());
                // Absolute amount tolerance allows for rounding in index-level ratios.
                QL_CHECK_SMALL(flow->amount() - amounts[i], 1.0e-8);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testEquityPaymentFrequenciesAndDates) {
    BOOST_TEST_MESSAGE("Testing equity payment frequencies and dates...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();
    // These dates include a two-business-day payment lag after adjusted period ends.
    const std::vector<Date> fundingPaymentDates = {
        Date(3, May, 2023), Date(2, August, 2023),
        Date(2, November, 2023), Date(2, February, 2024)};
    const std::vector<Date> semiannualDates = {
        Date(31, January, 2023), Date(31, July, 2023), Date(31, January, 2024)};
    for (bool overnight : {false, true}) {
        // Omitting the final constructor argument must retain maturity-only settlement.
        auto legacy = vars.createTRS(Swap::Receiver, schedule, overnight, 0.001,
                                     100000.0, 1.0, 2);
        auto maturity = vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight,
                                                   Once, 0.001, 2);
        BOOST_CHECK_EQUAL(legacy->equityPaymentFrequency(), Once);
        BOOST_CHECK_EQUAL(maturity->equityPaymentFrequency(), Once);
        BOOST_REQUIRE_EQUAL(legacy->equityLeg().size(), 1);
        BOOST_REQUIRE_EQUAL(maturity->equityLeg().size(), 1);
        BOOST_CHECK_EQUAL(maturity->equityLeg()[0]->date(), fundingPaymentDates.back());
        QL_CHECK_SMALL(legacy->NPV() - maturity->NPV(), 1.0e-8);
        QL_CHECK_SMALL(legacy->equityLeg()[0]->amount() -
                       maturity->equityLeg()[0]->amount(), 1.0e-8);

        for (Frequency frequency : {Quarterly, Semiannual, Monthly}) {
            auto periodic = vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight,
                                                       frequency, 0.001, 2);
            // This regular one-year term gives 4, 2 or 12 equity payments respectively.
            BOOST_REQUIRE_EQUAL(periodic->equityLeg().size(), Size(frequency));
            BOOST_REQUIRE_EQUAL(periodic->interestRateLeg().size(), fundingPaymentDates.size());
            // The frequency affects only equity settlements. Funding dates and amounts
            // must agree with the original quarterly funding leg for every frequency.
            for (Size i = 0; i < fundingPaymentDates.size(); ++i) {
                BOOST_CHECK_EQUAL(periodic->interestRateLeg()[i]->date(), fundingPaymentDates[i]);
                QL_CHECK_SMALL(periodic->interestRateLeg()[i]->amount() -
                               legacy->interestRateLeg()[i]->amount(), 1.0e-8);
                if (frequency == Quarterly)
                    BOOST_CHECK_EQUAL(periodic->equityLeg()[i]->date(), fundingPaymentDates[i]);
            }
            // Semiannual fixing boundaries skip the intervening funding settlements.
            // Their delayed payments coincide with the second and fourth funding payments.
            if (frequency == Semiannual) {
                for (Size i = 0; i < 2; ++i) {
                    auto flow = ext::dynamic_pointer_cast<EquityCashFlow>(periodic->equityLeg()[i]);
                    BOOST_REQUIRE(flow);
                    BOOST_CHECK_EQUAL(flow->baseDate(), semiannualDates[i]);
                    BOOST_CHECK_EQUAL(flow->fixingDate(), semiannualDates[i + 1]);
                    BOOST_CHECK_EQUAL(flow->date(), fundingPaymentDates[2 * i + 1]);
                }
            }
        }

        // Even a semiannual frequency gives one payment when the swap is shorter
        // than six months. This explicit-date schedule also tests missing metadata.
        const Schedule singlePeriod(std::vector<Date>{schedule[0], schedule[1]}, vars.calendar,
                                    Following);
        auto single = vars.createTRSWithFrequency(Swap::Receiver, singlePeriod, overnight,
                                                 Semiannual, 0.001, 2);
        auto bullet = vars.createTRSWithFrequency(Swap::Receiver, singlePeriod, overnight,
                                                 Once, 0.001, 2);
        BOOST_REQUIRE_EQUAL(single->equityLeg().size(), 1);
        BOOST_CHECK_EQUAL(single->equityLeg()[0]->date(), bullet->equityLeg()[0]->date());
        QL_CHECK_SMALL(single->NPV() - bullet->NPV(), 1.0e-8);

        // Unspecified frequencies must fail rather than silently select maturity settlement.
        for (Frequency frequency : {NoFrequency, OtherFrequency}) {
            BOOST_CHECK_THROW(vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight,
                                                          frequency), Error);
        }
    }
}

BOOST_AUTO_TEST_CASE(testPeriodicValuation) {
    BOOST_TEST_MESSAGE("Testing periodic valuation...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();

    // All equity period boundaries follow the evaluation date, so levels are forecast.
    // List expected equity dates independently of the instrument's schedule builder.
    // The monthly dates include weekend and New Year holiday adjustments.
    const std::vector<Date> quarterlyDates = {
        Date(31, January, 2023), Date(1, May, 2023), Date(31, July, 2023),
        Date(31, October, 2023), Date(31, January, 2024)};
    const std::vector<Date> semiannualDates = {
        Date(31, January, 2023), Date(31, July, 2023), Date(31, January, 2024)};
    const std::vector<Date> monthlyDates = {
        Date(31, January, 2023), Date(28, February, 2023), Date(31, March, 2023),
        Date(1, May, 2023), Date(31, May, 2023), Date(30, June, 2023),
        Date(31, July, 2023), Date(31, August, 2023), Date(2, October, 2023),
        Date(31, October, 2023), Date(30, November, 2023), Date(2, January, 2024),
        Date(31, January, 2024)};

    for (bool overnight : {false, true}) {
        for (Swap::Type type : {Swap::Receiver, Swap::Payer}) {
            for (Frequency frequency : {Quarterly, Semiannual, Monthly}) {
                const auto& dates = frequency == Quarterly ? quarterlyDates :
                                    frequency == Semiannual ? semiannualDates : monthlyDates;
                auto swap = vars.createTRSWithFrequency(type, schedule, overnight,
                                                       frequency, 0.001, 2);
                BOOST_REQUIRE_EQUAL(swap->equityLeg().size(), dates.size() - 1);
                // Sum returns over equity periods even when funding has a different frequency.
                Real expectedEquityNPV = 0.0;
                for (Size i = 1; i < dates.size(); ++i) {
                    // Flat continuously compounded carry is r - q. Value each return
                    // analytically rather than reusing EquityCashFlow::amount().
                    const Time dt = vars.dayCount.yearFraction(dates[i - 1], dates[i]);
                    // The lag changes discounting time, but not the return measurement period.
                    const Date payment = vars.calendar.advance(dates[i], 2, Days, Following);
                    BOOST_CHECK_EQUAL(swap->equityLeg()[i - 1]->date(), payment);
                    expectedEquityNPV += 100000.0 * (std::exp((0.0375 - 0.005) * dt) - 1.0)
                        * vars.interestHandle->discount(payment);
                }
                // Receiver/payer refers to equity; the funding leg carries the opposite sign.
                const Real sign = type == Swap::Receiver ? Real(1.0) : Real(-1.0);
                // Currency-valued comparisons allow 1e-8 absolute rounding error.
                QL_CHECK_SMALL(swap->equityLegNPV() - sign * expectedEquityNPV, 1.0e-8);

                // Check funding-leg signs, total NPV and the margin that reprices
                // the swap to par for both payer and receiver equity positions.
                QL_CHECK_SMALL(swap->interestRateLegNPV() +
                               sign * legNPV(swap->interestRateLeg(), vars.interestHandle), 1.0e-8);
                QL_CHECK_SMALL(swap->NPV() - swap->equityLegNPV() -
                               swap->interestRateLegNPV(), 1.0e-8);
                // Rebuild with the same equity frequency when checking the solved margin.
                auto par = vars.createTRSWithFrequency(type, schedule, overnight,
                                                      frequency, swap->fairMargin(), 2);
                QL_CHECK_SMALL(par->NPV(), 1.0e-8);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testNotionalResetCashFlows) {
    BOOST_TEST_MESSAGE("Testing equity cash flows with notional reset...");
    CommonVars vars;
    const std::vector<Date> dates = {Date(5, January, 2023), Date(5, April, 2023),
                                     Date(5, July, 2023), Date(5, October, 2023),
                                     Date(5, January, 2024)};
    const Schedule schedule(dates, vars.calendar, Unadjusted, Unadjusted,
                            3 * Months, DateGeneration::Forward, false);
    // Put every equity fixing in the past so amounts depend only on the supplied levels.
    Settings::instance().evaluationDate() = Date(8, January, 2024);

    const Real nominal = 100000.0;
    const Real levels[] = {100.0, 110.0, 99.0, 118.8, 118.8};
    for (Size i = 0; i < dates.size(); ++i)
        vars.equityIndex->addFixing(dates[i], levels[i], true);

    // Without reset, each period pays the return on the same notional; with reset,
    // the notional follows the index, i.e., a fixed number of index units is held.
    const std::vector<Real> fixedAmounts = {10000.0, -10000.0, 20000.0, 0.0};
    const std::vector<Real> resetAmounts = {10000.0, -11000.0, 19800.0, 0.0};
    const std::vector<Real> resetNotionals = {100000.0, 110000.0, 99000.0, 118800.0};

    for (bool overnight : {false, true}) {
        auto fixed = vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, Quarterly);
        auto reset = vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, Quarterly,
                                                 0.0, 0, Calendar(), true);
        BOOST_CHECK(!fixed->notionalReset());
        BOOST_CHECK(reset->notionalReset());
        BOOST_CHECK_EQUAL(reset->nominal(), nominal);
        BOOST_REQUIRE_EQUAL(fixed->equityLeg().size(), 4);
        BOOST_REQUIRE_EQUAL(reset->equityLeg().size(), 4);
        BOOST_REQUIRE_EQUAL(reset->interestRateLeg().size(), 4);

        Real total = 0.0;
        for (Size i = 0; i < 4; ++i) {
            QL_CHECK_SMALL(fixed->equityLeg()[i]->amount() - fixedAmounts[i], 1.0e-8);
            BOOST_CHECK(!ext::dynamic_pointer_cast<EquityNotionalResetCashFlow>(
                fixed->equityLeg()[i]));
            BOOST_CHECK(!ext::dynamic_pointer_cast<EquityNotionalResetCoupon>(
                fixed->interestRateLeg()[i]));

            auto flow =
                ext::dynamic_pointer_cast<EquityNotionalResetCashFlow>(reset->equityLeg()[i]);
            BOOST_REQUIRE(flow);
            BOOST_CHECK_EQUAL(flow->initialDate(), dates[0]);
            BOOST_CHECK_EQUAL(flow->baseDate(), dates[i]);
            BOOST_CHECK_EQUAL(flow->fixingDate(), dates[i + 1]);
            BOOST_CHECK_EQUAL(flow->date(), dates[i + 1]);
            BOOST_CHECK(flow->growthOnly());
            BOOST_CHECK_EQUAL(flow->initialNotional(), nominal);
            QL_CHECK_SMALL(flow->notional() - resetNotionals[i], 1.0e-8);
            QL_CHECK_SMALL(flow->amount() - resetAmounts[i], 1.0e-8);
            total += flow->amount();

            // The funding coupon accrues on the same notional as the equity cash flow.
            auto coupon =
                ext::dynamic_pointer_cast<EquityNotionalResetCoupon>(reset->interestRateLeg()[i]);
            BOOST_REQUIRE(coupon);
            BOOST_CHECK_EQUAL(coupon->initialDate(), dates[0]);
            BOOST_CHECK_EQUAL(coupon->resetDate(), dates[i]);
            BOOST_CHECK_EQUAL(coupon->accrualStartDate(), dates[i]);
            BOOST_CHECK_EQUAL(coupon->accrualEndDate(), dates[i + 1]);
            BOOST_CHECK_EQUAL(coupon->initialNominal(), nominal);
            BOOST_CHECK_EQUAL(coupon->underlying()->nominal(), nominal);
            QL_CHECK_SMALL(coupon->nominal() - resetNotionals[i], 1.0e-8);
        }
        // The payments add up to the return of buying and holding the index.
        QL_CHECK_SMALL(total - nominal * (levels[4] / levels[0] - 1.0), 1.0e-8);
    }
}

BOOST_AUTO_TEST_CASE(testNotionalResetValuation) {
    BOOST_TEST_MESSAGE("Testing valuation with notional reset...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();
    // All period boundaries follow the evaluation date, so that all levels are forecast.
    const std::vector<Date> dates = {
        Date(31, January, 2023), Date(1, May, 2023), Date(31, July, 2023),
        Date(31, October, 2023), Date(31, January, 2024)};
    const Real nominal = 100000.0;
    const Rate margin = 0.001;

    for (bool overnight : {false, true}) {
        for (Swap::Type type : {Swap::Receiver, Swap::Payer}) {
            auto fixed = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly,
                                                     margin, 2);
            auto reset = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly,
                                                     margin, 2, Calendar(), true);
            BOOST_REQUIRE_EQUAL(reset->equityLeg().size(), dates.size() - 1);
            BOOST_REQUIRE_EQUAL(reset->interestRateLeg().size(), dates.size() - 1);

            Real expectedEquityNPV = 0.0, expectedInterestNPV = 0.0;
            for (Size i = 1; i < dates.size(); ++i) {
                // With flat continuously compounded carry r - q, the index forecast
                // relative to the start date is known analytically.
                const Real growthToStart = std::exp(
                    (0.0375 - 0.005) * vars.dayCount.yearFraction(dates[0], dates[i - 1]));
                const Real growthToEnd = std::exp(
                    (0.0375 - 0.005) * vars.dayCount.yearFraction(dates[0], dates[i]));
                const Date payment = vars.calendar.advance(dates[i], 2, Days, Following);
                const DiscountFactor discount = vars.interestHandle->discount(payment);

                const auto& flow = reset->equityLeg()[i - 1];
                BOOST_CHECK_EQUAL(flow->date(), payment);
                const Real expectedAmount = nominal * (growthToEnd - growthToStart);
                QL_CHECK_SMALL(flow->amount() - expectedAmount, 1.0e-8);
                expectedEquityNPV += expectedAmount * discount;

                // The forecast funding coupon is the product of the forecast notional
                // and the forecast rate; the latter is the one of the fixed-notional swap.
                auto coupon = ext::dynamic_pointer_cast<Coupon>(reset->interestRateLeg()[i - 1]);
                auto fixedCoupon =
                    ext::dynamic_pointer_cast<Coupon>(fixed->interestRateLeg()[i - 1]);
                BOOST_REQUIRE(coupon);
                BOOST_REQUIRE(fixedCoupon);
                BOOST_CHECK_EQUAL(coupon->date(), payment);
                QL_CHECK_SMALL(coupon->nominal() - nominal * growthToStart, 1.0e-8);
                QL_CHECK_CLOSE(coupon->rate(), fixedCoupon->rate(), 1.0e-10);
                QL_CHECK_SMALL(coupon->amount() - fixedCoupon->amount() * growthToStart,
                               1.0e-8);
                expectedInterestNPV += fixedCoupon->amount() * growthToStart * discount;
            }

            // Receiver/payer refers to equity; the funding leg carries the opposite sign.
            const Real sign = type == Swap::Receiver ? Real(1.0) : Real(-1.0);
            QL_CHECK_SMALL(reset->equityLegNPV() - sign * expectedEquityNPV, 1.0e-8);
            QL_CHECK_SMALL(reset->interestRateLegNPV() + sign * expectedInterestNPV, 1.0e-8);
            QL_CHECK_SMALL(reset->NPV() - reset->equityLegNPV() - reset->interestRateLegNPV(),
                           1.0e-8);

            // The fair margin accrues on the reset notionals.
            auto par = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly,
                                                   reset->fairMargin(), 2, Calendar(), true);
            QL_CHECK_SMALL(par->NPV(), 1.0e-8);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSeasonedNotionalReset) {
    BOOST_TEST_MESSAGE("Testing seasoned swap with notional reset...");
    CommonVars vars;
    // The swap started before the evaluation date: the initial index level is a
    // past fixing, while the levels at the following resets are forecast.
    const Schedule schedule = MakeSchedule().from(Date(5, January, 2023))
        .to(Date(5, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();
    const Real nominal = 100000.0;
    const Real initialLevel = 9010.0;

    for (bool overnight : {false, true}) {
        for (Swap::Type type : {Swap::Receiver, Swap::Payer}) {
            auto bullet = vars.createTRSWithFrequency(type, schedule, overnight, Once);
            auto fixed = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly);
            auto reset = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly,
                                                     0.0, 0, Calendar(), true);
            const Size n = schedule.size() - 1;
            BOOST_REQUIRE_EQUAL(reset->equityLeg().size(), n);
            BOOST_REQUIRE_EQUAL(reset->interestRateLeg().size(), n);

            auto check = [&]() {
                Real total = 0.0, expectedEquityNPV = 0.0, expectedInterestNPV = 0.0;
                for (Size i = 1; i <= n; ++i) {
                    const Real startLevel = vars.equityIndex->fixing(schedule[i - 1]);
                    const Real endLevel = vars.equityIndex->fixing(schedule[i]);
                    const Real notional = nominal * startLevel / initialLevel;
                    const DiscountFactor discount = vars.interestHandle->discount(schedule[i]);

                    auto flow = ext::dynamic_pointer_cast<EquityCashFlow>(
                        reset->equityLeg()[i - 1]);
                    BOOST_REQUIRE(flow);
                    QL_CHECK_SMALL(flow->notional() - notional, 1.0e-8);
                    QL_CHECK_SMALL(flow->amount() -
                                   nominal * (endLevel - startLevel) / initialLevel, 1.0e-8);
                    total += flow->amount();
                    expectedEquityNPV += flow->amount() * discount;

                    auto coupon =
                        ext::dynamic_pointer_cast<Coupon>(reset->interestRateLeg()[i - 1]);
                    auto fixedCoupon =
                        ext::dynamic_pointer_cast<Coupon>(fixed->interestRateLeg()[i - 1]);
                    BOOST_REQUIRE(coupon);
                    BOOST_REQUIRE(fixedCoupon);
                    QL_CHECK_SMALL(coupon->nominal() - notional, 1.0e-8);
                    QL_CHECK_SMALL(coupon->amount() -
                                   fixedCoupon->amount() * startLevel / initialLevel, 1.0e-8);
                    expectedInterestNPV += coupon->amount() * discount;
                }
                // The first period started in the past and keeps the initial notional.
                auto first = ext::dynamic_pointer_cast<Coupon>(reset->interestRateLeg()[0]);
                QL_CHECK_SMALL(first->nominal() - nominal, 1.0e-8);
                // The payments add up to the single payment of the swap settled at maturity.
                QL_CHECK_SMALL(total - bullet->equityLeg()[0]->amount(), 1.0e-8);

                const Real sign = type == Swap::Receiver ? Real(1.0) : Real(-1.0);
                QL_CHECK_SMALL(reset->equityLegNPV() - sign * expectedEquityNPV, 1.0e-8);
                QL_CHECK_SMALL(reset->interestRateLegNPV() + sign * expectedInterestNPV,
                               1.0e-8);
            };
            check();

            auto par = vars.createTRSWithFrequency(type, schedule, overnight, Quarterly,
                                                   reset->fairMargin(), 0, Calendar(), true);
            QL_CHECK_SMALL(par->NPV(), 1.0e-8);

            // Both legs must react to changes in the inputs of the equity forecast.
            const Real equityNPV = reset->equityLegNPV();
            const Real interestNPV = reset->interestRateLegNPV();
            auto spotQuote = ext::dynamic_pointer_cast<SimpleQuote>(vars.spot);
            spotQuote->setValue(9100.0);
            BOOST_CHECK(std::fabs(reset->equityLegNPV() - equityNPV) > 1.0);
            BOOST_CHECK(std::fabs(reset->interestRateLegNPV() - interestNPV) > 1.0);
            check();

            const Real bumpedEquityNPV = reset->equityLegNPV();
            const Real bumpedInterestNPV = reset->interestRateLegNPV();
            vars.dividendHandle.linkTo(flatRate(0.02, vars.dayCount));
            BOOST_CHECK(std::fabs(reset->equityLegNPV() - bumpedEquityNPV) > 1.0);
            BOOST_CHECK(std::fabs(reset->interestRateLegNPV() - bumpedInterestNPV) > 1.0e-2);
            check();

            spotQuote->setValue(8700.0);
            vars.dividendHandle.linkTo(flatRate(0.005, vars.dayCount));
        }
    }
}

BOOST_AUTO_TEST_CASE(testErrorWhenNotionalResetIsNotSupported) {
    BOOST_TEST_MESSAGE("Testing error when notional reset is not supported...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();

    for (bool overnight : {false, true}) {
        // The notional cannot be reset if the equity return is settled once at maturity...
        BOOST_CHECK_EXCEPTION(
            vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, Once, 0.0, 0,
                                        Calendar(), true),
            Error, ExpectedErrorMessage("Notional reset requires periodic equity payments"));
        // ...or if the equity periods differ from the funding periods.
        for (Frequency frequency : {Semiannual, Monthly}) {
            BOOST_CHECK_EXCEPTION(
                vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, frequency, 0.0,
                                            0, Calendar(), true),
                Error,
                ExpectedErrorMessage(
                    "Notional reset requires equity periods matching the funding periods"));
        }
        // Without reset, the same frequencies are still allowed.
        BOOST_CHECK_NO_THROW(
            vars.createTRSWithFrequency(Swap::Receiver, schedule, overnight, Semiannual));
    }
}

BOOST_AUTO_TEST_CASE(testNotionalResetFundingCurveUpdate) {
    BOOST_TEST_MESSAGE("Testing notional reset after funding curve changes...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();

    // The funding index is forecast on a different curve than the discount curve,
    // so that the swap is only notified of its changes through the funding coupons.
    RelinkableHandle<YieldTermStructure> forecastHandle(flatRate(0.04, vars.dayCount));
    // declared as base classes to select the right constructor of the swap
    ext::shared_ptr<IborIndex> libor = ext::make_shared<USDLibor>(3 * Months, forecastHandle);
    ext::shared_ptr<OvernightIndex> sofr = ext::make_shared<Sofr>(forecastHandle);

    auto makeSwap = [&](bool overnight) {
        ext::shared_ptr<EquityTotalReturnSwap> swap;
        if (overnight)
            swap = ext::make_shared<EquityTotalReturnSwap>(
                Swap::Receiver, 100000.0, schedule, vars.equityIndex, sofr, vars.dayCount, 0.0,
                1.0, vars.calendar, Following, 0, Quarterly, true);
        else
            swap = ext::make_shared<EquityTotalReturnSwap>(
                Swap::Receiver, 100000.0, schedule, vars.equityIndex, libor, vars.dayCount, 0.0,
                1.0, vars.calendar, Following, 0, Quarterly, true);
        swap->setPricingEngine(vars.discountEngine);
        return swap;
    };

    for (bool overnight : {false, true}) {
        auto swap = makeSwap(overnight);
        const Real interestNPV = swap->interestRateLegNPV();
        forecastHandle.linkTo(flatRate(0.05, vars.dayCount));
        BOOST_CHECK(std::fabs(swap->interestRateLegNPV() - interestNPV) > 1.0);
        QL_CHECK_SMALL(swap->interestRateLegNPV() - makeSwap(overnight)->interestRateLegNPV(),
                       1.0e-8);
        forecastHandle.linkTo(flatRate(0.04, vars.dayCount));
    }
}

BOOST_AUTO_TEST_CASE(testFairMargin) {
    BOOST_TEST_MESSAGE("Testing fair margin...");

    // Check TRS vs Libor-type index
    checkFairMarginCalculation(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), false);
    checkFairMarginCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), false,
                               0.01);
    checkFairMarginCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), false,
                               0.0, 0.0);
    checkFairMarginCalculation(Swap::Receiver, Date(31, January, 2023), Date(30, April, 2023),
                               false, -0.005, 1.0, 2);

    // Check TRS vs overnight index
    checkFairMarginCalculation(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), true);
    checkFairMarginCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), true,
                               0.01);
    checkFairMarginCalculation(Swap::Receiver, Date(31, January, 2023), Date(30, April, 2023), true,
                               -0.005, 1.0, 2);
}

BOOST_AUTO_TEST_CASE(testErrorWhenNegativeNominal) {
    BOOST_TEST_MESSAGE("Testing error when negative nominal...");

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.createTRS(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), false, 0.0,
                       -1.e7),
        Error,
        ExpectedErrorMessage("Nominal cannot be negative"));
}

BOOST_AUTO_TEST_CASE(testErrorWhenNoPaymentCalendar) {
    BOOST_TEST_MESSAGE("Testing error when payment calendar is missing...");

    CommonVars vars;
    
    auto sch = Schedule(Date(5, January, 2023), Date(5, April, 2023), 3 * Months, Calendar(),
                        Unadjusted, Unadjusted, DateGeneration::Rule::Backward, false);

    BOOST_CHECK_EXCEPTION(
        vars.createTRS(Swap::Receiver, sch, false), Error,
        ExpectedErrorMessage("Calendar in schedule cannot be empty"));
}

BOOST_AUTO_TEST_CASE(testEquityLegNPV) {
    BOOST_TEST_MESSAGE("Testing equity leg NPV replication...");

    CommonVars vars;

    const Real tolerance = 1.0e-8;

    Date start(5, January, 2023);
    Date end(5, April, 2023);

    auto trs = vars.createTRS(Swap::Receiver, start, end, false);
    auto actualEquityLegNPV = trs->equityLegNPV();

    auto eqIdx = trs->equityIndex();
    auto discount = vars.interestHandle->discount(end);
    auto expectedEquityLegNPV =
        (eqIdx->fixing(end) / eqIdx->fixing(start) - 1.0) * trs->nominal() * discount;

    if ((std::fabs(actualEquityLegNPV - expectedEquityLegNPV) > tolerance))
        BOOST_ERROR("unable to replicate equity leg NPV\n"
                    << "    actual NPV:    " << actualEquityLegNPV << "\n"
                    << "    expected NPV:    " << expectedEquityLegNPV << "\n");
}

BOOST_AUTO_TEST_CASE(testTRSNPV) {
    BOOST_TEST_MESSAGE("Testing TRS NPV...");

    CommonVars vars;

    // Check TRS vs Libor-type index
    checkNPVCalculation(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), false);
    checkNPVCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), false, 0.01);
    checkNPVCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), false, 0.0, 0.0);
    checkNPVCalculation(Swap::Receiver, Date(31, January, 2023), Date(30, April, 2023), false,
                        -0.005, 1.0, 2);

    //// Check TRS vs overnight index
    checkNPVCalculation(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), true);
    checkNPVCalculation(Swap::Payer, Date(5, January, 2023), Date(5, April, 2023), true, 0.01);
    checkNPVCalculation(Swap::Receiver, Date(31, January, 2023), Date(30, April, 2023), true,
                        -0.005, 1.0, 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

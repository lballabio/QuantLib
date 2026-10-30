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

    ext::shared_ptr<EquityTotalReturnSwap> createTRSWithMode(
        Swap::Type type, const Schedule& schedule, bool useOvernightIndex,
        EquityTotalReturnSwap::EquityPaymentMode mode, Rate margin = 0.0,
        Natural paymentDelay = 0, Calendar paymentCalendar = Calendar()) {
        if (paymentCalendar.empty())
            paymentCalendar = schedule.calendar();
        ext::shared_ptr<EquityTotalReturnSwap> swap;
        if (useOvernightIndex) {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                type, 100000.0, schedule, equityIndex, sofr, dayCount, margin, mode,
                1.0, paymentCalendar, Following, paymentDelay);
        } else {
            swap = ext::make_shared<EquityTotalReturnSwap>(
                type, 100000.0, schedule, equityIndex, usdLibor, dayCount, margin, mode,
                1.0, paymentCalendar, Following, paymentDelay);
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
    CommonVars vars;
    const std::vector<Date> dates = {Date(5, January, 2023), Date(5, April, 2023),
                                     Date(5, July, 2023), Date(5, October, 2023),
                                     Date(5, January, 2024)};
    const Schedule schedule(dates, vars.calendar, Unadjusted, Unadjusted,
                            3 * Months, DateGeneration::Forward, false);
    Settings::instance().evaluationDate() = Date(8, January, 2024);
    const Real levels[] = {100.0, 110.0, 99.0, 118.8, 118.8};
    const Real amounts[] = {10000.0, -10000.0, 20000.0, 0.0};
    for (Size i = 0; i < dates.size(); ++i)
        vars.equityIndex->addFixing(dates[i], levels[i], true);

    for (bool overnight : {false, true}) {
        auto swap = vars.createTRSWithMode(Swap::Receiver, schedule, overnight,
                                           EquityTotalReturnSwap::Periodic);
        BOOST_CHECK_EQUAL(swap->equityPaymentMode(), EquityTotalReturnSwap::Periodic);
        BOOST_REQUIRE_EQUAL(swap->equityLeg().size(), 4);
        BOOST_REQUIRE_EQUAL(swap->interestRateLeg().size(), 4);
        for (Size i = 0; i < 4; ++i) {
            auto flow = ext::dynamic_pointer_cast<EquityCashFlow>(swap->equityLeg()[i]);
            BOOST_REQUIRE(flow);
            BOOST_CHECK_EQUAL(flow->baseDate(), dates[i]);
            BOOST_CHECK_EQUAL(flow->fixingDate(), dates[i + 1]);
            BOOST_CHECK_EQUAL(flow->date(), dates[i + 1]);
            BOOST_CHECK_EQUAL(flow->notional(), 100000.0);
            BOOST_CHECK(flow->growthOnly());
            BOOST_CHECK_SMALL(flow->amount() - amounts[i], 1.0e-8);
        }
    }
}

BOOST_AUTO_TEST_CASE(testEquityPaymentModesAndDates) {
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();
    const std::vector<Date> paymentDates = {Date(3, May, 2023), Date(2, August, 2023),
                                           Date(2, November, 2023), Date(2, February, 2024)};
    for (bool overnight : {false, true}) {
        auto legacy = vars.createTRS(Swap::Receiver, schedule, overnight, 0.001,
                                     100000.0, 1.0, 2);
        auto maturity = vars.createTRSWithMode(Swap::Receiver, schedule, overnight,
                                               EquityTotalReturnSwap::AtMaturity, 0.001, 2);
        BOOST_CHECK_EQUAL(legacy->equityPaymentMode(), EquityTotalReturnSwap::AtMaturity);
        BOOST_REQUIRE_EQUAL(legacy->equityLeg().size(), 1);
        BOOST_REQUIRE_EQUAL(maturity->equityLeg().size(), 1);
        BOOST_CHECK_EQUAL(maturity->equityLeg()[0]->date(), paymentDates.back());
        BOOST_CHECK_SMALL(legacy->NPV() - maturity->NPV(), 1.0e-8);
        BOOST_CHECK_SMALL(legacy->equityLeg()[0]->amount() -
                          maturity->equityLeg()[0]->amount(), 1.0e-8);

        {
            const Calendar& cal = vars.calendar;
            auto periodic = vars.createTRSWithMode(Swap::Receiver, schedule, overnight,
                                                   EquityTotalReturnSwap::Periodic, 0.001, 2, cal);
            BOOST_REQUIRE_EQUAL(periodic->equityLeg().size(), paymentDates.size());
            for (Size i = 0; i < paymentDates.size(); ++i) {
                BOOST_CHECK_EQUAL(periodic->equityLeg()[i]->date(), paymentDates[i]);
                BOOST_CHECK_EQUAL(periodic->interestRateLeg()[i]->date(), paymentDates[i]);
                BOOST_CHECK_SMALL(periodic->interestRateLeg()[i]->amount() -
                                  legacy->interestRateLeg()[i]->amount(), 1.0e-8);
            }
        }
        const Schedule singlePeriod(std::vector<Date>{schedule[0], schedule[1]}, vars.calendar,
                                    Following, Following, 3 * Months, DateGeneration::Forward, false);
        auto single = vars.createTRSWithMode(Swap::Receiver, singlePeriod, overnight,
                                             EquityTotalReturnSwap::Periodic, 0.001, 2);
        auto bullet = vars.createTRSWithMode(Swap::Receiver, singlePeriod, overnight,
                                             EquityTotalReturnSwap::AtMaturity, 0.001, 2);
        BOOST_REQUIRE_EQUAL(single->equityLeg().size(), 1);
        BOOST_CHECK_SMALL(single->NPV() - bullet->NPV(), 1.0e-8);
    }
}

BOOST_AUTO_TEST_CASE(testPeriodicValuation) {
    BOOST_TEST_MESSAGE("Testing periodic valuation...");
    CommonVars vars;
    const Schedule schedule = MakeSchedule().from(Date(31, January, 2023))
        .to(Date(31, January, 2024)).withTenor(3 * Months)
        .withCalendar(vars.calendar).withConvention(Following).backwards();

    BOOST_TEST_MESSAGE("Evaluation date: "<< Settings::instance().evaluationDate());
    BOOST_TEST_MESSAGE("Discount curve reference date:"<< vars.interestHandle->referenceDate());

    //select overnight-index or term-index financing
    for (bool overnight : {false, true}) {
        for (Swap::Type type : {Swap::Receiver, Swap::Payer}) {

            //Requests periodic equity settlements. Instead of accumulating the entire equity return
            //into one payment at maturity, the implementation should create equity payments for each period.
            auto swap = vars.createTRSWithMode(type, schedule, overnight,
                                               EquityTotalReturnSwap::Periodic, 0.001, 2);
            Real expectedEquityNPV = 0.0;
            for (Size i = 1; i < schedule.size(); ++i) {
                // Flat carry is r - q; calculate independently of cash-flow amount().
                // The analytical model assumes a flat continuously compounded equity carry
                //dt is expressed in fraction of years
                Time dt = vars.dayCount.yearFraction(schedule[i - 1], schedule[i]);

                //calculate the equity payment date. Here, the delay is two business days.
                Date payment = vars.calendar.advance(schedule[i], 2, Days, Following);

                //calculate the periodic equity return, discount the equity cash flow
                expectedEquityNPV += 100000.0 * (std::exp((0.0375 - 0.005) * dt) - 1.0)
                    * vars.interestHandle->discount(payment);
            }
            Real sign = type == Swap::Receiver ? 1.0 : -1.0;
            BOOST_CHECK_SMALL(swap->equityLegNPV() - sign * expectedEquityNPV, 1.0e-8);

            //The legNPV helper values the financing-leg cash flows that the swap implementation has already constructed.
            BOOST_CHECK_SMALL(swap->interestRateLegNPV() +
                              sign * legNPV(swap->interestRateLeg(), vars.interestHandle), 1.0e-8);

            //SwapNPV - EquityLegNPV - InterestRateLegNPV = 0
            BOOST_CHECK_SMALL(swap->NPV() - swap->equityLegNPV() -
                              swap->interestRateLegNPV(), 1.0e-8);

            auto par = vars.createTRSWithMode(type, schedule, overnight,
                                              EquityTotalReturnSwap::Periodic, swap->fairMargin(), 2);
            BOOST_CHECK_SMALL(par->NPV(), 1.0e-8);
        }
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

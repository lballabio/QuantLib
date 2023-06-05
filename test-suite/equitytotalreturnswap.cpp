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

#include "equitytotalreturnswap.hpp"
#include "utilities.hpp"
#include <ql/instruments/equitytotalreturnswap.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace equitytotalreturnswap_test {

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

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            calendar = TARGET();
            dayCount = Actual365Fixed();

            today = calendar.adjust(Date(27, January, 2023));
            Settings::instance().evaluationDate() = today;

            equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, interestHandle,
                                                        dividendHandle, spotHandle);
            IndexManager::instance().clearHistory(equityIndex->name());
            equityIndex->addFixing(Date(5, January, 2023), 9010.0);
            equityIndex->addFixing(today, 8690.0);

            sofr = ext::make_shared<Sofr>(interestHandle);
            IndexManager::instance().clearHistory(sofr->name());
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
            IndexManager::instance().clearHistory(usdLibor->name());
            usdLibor->addFixing(Date(3, January, 2023), 0.035);

            interestHandle.linkTo(flatRate(0.0375, dayCount));
            dividendHandle.linkTo(flatRate(0.005, dayCount));

            discountEngine =
                ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(interestHandle));

            spot = ext::make_shared<SimpleQuote>(8700.0);
            spotHandle.linkTo(spot);
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
}

void EquityTotalReturnSwapTest::testFairMargin() {
    BOOST_TEST_MESSAGE("Testing fair margin...");

    using namespace equitytotalreturnswap_test;

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

void EquityTotalReturnSwapTest::testErrorWhenNegativeNominal() {
    BOOST_TEST_MESSAGE("Testing error when negative nominal...");

    using namespace equitytotalreturnswap_test;

    CommonVars vars;

    BOOST_CHECK_EXCEPTION(
        vars.createTRS(Swap::Receiver, Date(5, January, 2023), Date(5, April, 2023), false, 0.0,
                       -1.e7),
        Error,
        ExpectedErrorMessage("Nominal cannot be negative"));
}

void EquityTotalReturnSwapTest::testErrorWhenNoPaymentCalendar() {
    BOOST_TEST_MESSAGE("Testing error when payment calendar is missing...");

    using namespace equitytotalreturnswap_test;

    CommonVars vars;
    
    auto sch = Schedule(Date(5, January, 2023), Date(5, April, 2023), 3 * Months, Calendar(),
                        Unadjusted, Unadjusted, DateGeneration::Rule::Backward, false);

    BOOST_CHECK_EXCEPTION(
        vars.createTRS(Swap::Receiver, sch, false), Error,
        ExpectedErrorMessage("Calendar in schedule cannot be empty"));
}

void EquityTotalReturnSwapTest::testEquityLegNPV() {
    BOOST_TEST_MESSAGE("Testing equity leg NPV replication...");

    using namespace equitytotalreturnswap_test;

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

void EquityTotalReturnSwapTest::testTRSNPV() {
    BOOST_TEST_MESSAGE("Testing TRS NPV...");

    using namespace equitytotalreturnswap_test;

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

test_suite* EquityTotalReturnSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity total return swap tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testFairMargin));
    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testErrorWhenNegativeNominal));
    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testErrorWhenNoPaymentCalendar));
    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testEquityLegNPV));
    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testTRSNPV));

    return suite;
}

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
#include <string>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace equitytotalreturnswap_test {

	// Used to check that the exception message contains the expected message string, expMsg.
    struct ExpErrorPred {

        explicit ExpErrorPred(std::string msg) : expMsg(std::move(msg)) {}

        bool operator()(const Error& ex) const {
            std::string errMsg(ex.what());
            if (errMsg.find(expMsg) == std::string::npos) {
                BOOST_TEST_MESSAGE("Error expected to contain: '" << expMsg << "'.");
                BOOST_TEST_MESSAGE("Actual error is: '" << errMsg << "'.");
                return false;
            } else {
                return true;
            }
        }

        std::string expMsg;
    };

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

            interestHandle.linkTo(flatRate(0.03, dayCount));
            dividendHandle.linkTo(flatRate(0.01, dayCount));

            discountEngine =
                ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(interestHandle));

            spot = ext::make_shared<SimpleQuote>(8700.0);
            spotHandle.linkTo(spot);
        }

        ext::shared_ptr<EquityTotalReturnSwap> createTRS(Swap::Type type,
                                                         const Date& start,
                                                         const Date& end,
                                                         bool useOvernightIndex,
                                                         Rate margin = 0.0,
                                                         Real nominal = 1.0e7,
                                                         Real gearing = 1.0,
                                                         Natural paymentDelay = 0) {
            auto schedule = MakeSchedule()
                                .from(start)
                                .to(end)
                                .withTenor(3 * Months)
                                .withCalendar(calendar)
                                .withConvention(Following)
                                .backwards();
            auto swap = ext::make_shared<EquityTotalReturnSwap>(
                type, nominal, schedule, equityIndex, useOvernightIndex ? sofr : usdLibor, dayCount,
                margin, gearing, calendar, Following, paymentDelay);
            swap->setPricingEngine(discountEngine);
            return swap;
        }
    };
}

void EquityTotalReturnSwapTest::testFairMargin() {
    BOOST_TEST_MESSAGE("Testing fair margin...");

    using namespace equitytotalreturnswap_test;

    CommonVars vars;

    const Real tolerance = 1.0e-8;

    Date start(5, January, 2023);
    Date end(5, April, 2023);

    auto trsVsLibor = vars.createTRS(Swap::Receiver, start, end, false);
    auto fairMargin = trsVsLibor->fairMargin();
    auto parTrsVsLibor = vars.createTRS(Swap::Receiver, start, end, false, fairMargin);
    
    auto trsVsSofr = vars.createTRS(Swap::Receiver, start, end, true);

    if ((std::fabs(parTrsVsLibor->NPV()) > tolerance))
        BOOST_ERROR("unable to replicate NPV\n"
                    << "    actual NPV:    " << parTrsVsLibor->NPV() << "\n"
                    << "    expected NPV:    " << fairMargin << "\n"
                    << "    expected NPV:    " << parTrsVsLibor->equityLegNPV() + parTrsVsLibor->interestRateLegNPV() << "\n");
}

test_suite* EquityTotalReturnSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity total return swap tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityTotalReturnSwapTest::testFairMargin));

    return suite;
}
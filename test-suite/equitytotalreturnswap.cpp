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
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>
#include <string>

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
        RelinkableHandle<YieldTermStructure> interestHandle;
        RelinkableHandle<YieldTermStructure> dividendHandle;
        ext::shared_ptr<Quote> spot;
        RelinkableHandle<Quote> spotHandle;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars(bool addTodaysFixing = true) {
            calendar = TARGET();
            dayCount = Actual365Fixed();

            equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, interestHandle,
                                                        dividendHandle, spotHandle);
            IndexManager::instance().clearHistory(equityIndex->name());

            today = calendar.adjust(Date(27, January, 2023));

            if (addTodaysFixing)
                equityIndex->addFixing(today, 8690.0);

            Settings::instance().evaluationDate() = today;

            interestHandle.linkTo(flatRate(0.03, dayCount));
            dividendHandle.linkTo(flatRate(0.01, dayCount));

            spot = ext::make_shared<SimpleQuote>(8700.0);
            spotHandle.linkTo(spot);
        }
    };
}
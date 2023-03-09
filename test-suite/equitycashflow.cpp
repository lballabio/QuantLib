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

#include "equitycashflow.hpp"
#include "utilities.hpp"
#include <ql/cashflows/equitycashflow.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace equitycashflow_test {

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

        Real notional;

        ext::shared_ptr<EquityIndex> equityIndex;
        
        RelinkableHandle<YieldTermStructure> localCcyInterestHandle;
        RelinkableHandle<YieldTermStructure> dividendHandle;
        RelinkableHandle<YieldTermStructure> quantoCcyInterestHandle;

        RelinkableHandle<BlackVolTermStructure> equityVolHandle;
        RelinkableHandle<BlackVolTermStructure> fxVolHandle;
        
        RelinkableHandle<Quote> spotHandle;
        RelinkableHandle<Quote> correlationHandle;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            calendar = TARGET();
            dayCount = Actual365Fixed();
            notional = 1.0e7;

            today = calendar.adjust(Date(27, January, 2023));
            Settings::instance().evaluationDate() = today;

            equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, localCcyInterestHandle,
                                                        dividendHandle, spotHandle);
            IndexManager::instance().clearHistory(equityIndex->name());
            equityIndex->addFixing(Date(5, January, 2023), 9010.0);
            equityIndex->addFixing(today, 8690.0);

            localCcyInterestHandle.linkTo(flatRate(0.0375, dayCount));
            dividendHandle.linkTo(flatRate(0.005, dayCount));
            quantoCcyInterestHandle.linkTo(flatRate(0.001, dayCount));

            equityVolHandle.linkTo(flatVol(0.4, dayCount));
            fxVolHandle.linkTo(flatVol(0.2, dayCount));

            spotHandle.linkTo(ext::make_shared<SimpleQuote>(8700.0));
            correlationHandle.linkTo(ext::make_shared<SimpleQuote>(0.4));
        }

        ext::shared_ptr<EquityCashFlow> createEquityQuantoCashFlow(const Date& start,
                                                                         const Date& end) {
            return ext::make_shared<EquityCashFlow>(notional, equityIndex, start, end, end);
        }

        ext::shared_ptr<EquityCashFlowPricer> createEquityQuantoPricer() {
            return ext::make_shared<EquityQuantoCashFlowPricer>(
                quantoCcyInterestHandle, equityVolHandle, fxVolHandle, correlationHandle);
        }
    };
}

void EquityCashFlowTest::testQuantoCorrection() {
    BOOST_TEST_MESSAGE("Testing quanto correction...");

    using namespace equitycashflow_test;

    const Real tolerance = 1.0e-6;

    CommonVars vars;

    Date startDate(5, January, 2023);
    Date endDate(5, April, 2023);

    auto cf = vars.createEquityQuantoCashFlow(startDate, endDate);
    auto pricer = vars.createEquityQuantoPricer();
    cf->setPricer(pricer);

    Real strike = vars.equityIndex->fixing(endDate);
    Real indexStart = vars.equityIndex->fixing(startDate);

    Real time = vars.localCcyInterestHandle->timeFromReference(endDate);
    Real rf = vars.localCcyInterestHandle->zeroRate(time, Continuous);
    Real q = vars.dividendHandle->zeroRate(time, Continuous);
    Real eqVol = vars.equityVolHandle->blackVol(endDate, strike);
    Real fxVol = vars.fxVolHandle->blackVol(endDate, 1.0);
    Real rho = vars.correlationHandle->value();
    Real spot = vars.spotHandle->value();
    
    Real quantoForward = spot * std::exp((rf - q - rho * eqVol * fxVol) * time);
    Real expectedAmount = (quantoForward / indexStart - 1.0) * vars.notional;

    Real actualAmount = cf->amount();

    if ((std::fabs(actualAmount - expectedAmount) > tolerance))
        BOOST_ERROR("could not replicate equity quanto correction\n"
                    << "    actual amount:    " << actualAmount << "\n"
                    << "    expected amount:    " << expectedAmount << "\n"
                    << "    local rate:    " << rf << "\n"
                    << "    equity volatility:    " << eqVol << "\n"
                    << "    FX volatility:    " << fxVol << "\n"
                    << "    correlation:    " << rho << "\n"
                    << "    spot:    " << spot << "\n");
}

test_suite* EquityCashFlowTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity quanto cash flow tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityCashFlowTest::testQuantoCorrection));

    return suite;
}
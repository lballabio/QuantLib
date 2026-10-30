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
#include <ql/cashflows/equitycashflow.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(EquityCashFlowTests)

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

    // utilities

    CommonVars() {
        calendar = TARGET();
        dayCount = Actual365Fixed();
        notional = 1.0e7;

        today = calendar.adjust(Date(27, January, 2023));
        Settings::instance().evaluationDate() = today;

        equityIndex = ext::make_shared<EquityIndex>("eqIndex", calendar, EURCurrency(), localCcyInterestHandle,
                                                    dividendHandle, spotHandle);
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

    ext::shared_ptr<EquityCashFlow>
    createEquityQuantoCashFlow(const ext::shared_ptr<EquityIndex>& index,
                               const Date& start,
                               const Date& end,
                               bool useQuantoPricer = true) {

        auto cf = ext::make_shared<EquityCashFlow>(notional, index, start, end, end);
        if (useQuantoPricer) {
            auto pricer = ext::make_shared<EquityQuantoCashFlowPricer>(
                    quantoCcyInterestHandle, equityVolHandle, fxVolHandle, correlationHandle);
            cf->setPricer(pricer);
        }
        return cf;
    }

    ext::shared_ptr<EquityCashFlow>
    createEquityQuantoCashFlow(const ext::shared_ptr<EquityIndex>& index,
                               bool useQuantoPricer = true) {
        Date start(5, January, 2023);
        Date end(5, April, 2023);

        return createEquityQuantoCashFlow(index, start, end, useQuantoPricer);
    }

    ext::shared_ptr<EquityCashFlow> createEquityQuantoCashFlow(bool useQuantoPricer = true) {
        return createEquityQuantoCashFlow(equityIndex, useQuantoPricer);
    }
};

void bumpMarketData(CommonVars& vars) {
        
    vars.localCcyInterestHandle.linkTo(flatRate(0.04, vars.dayCount));
    vars.dividendHandle.linkTo(flatRate(0.01, vars.dayCount));
    vars.quantoCcyInterestHandle.linkTo(flatRate(0.03, vars.dayCount));

    vars.equityVolHandle.linkTo(flatVol(0.45, vars.dayCount));
    vars.fxVolHandle.linkTo(flatVol(0.25, vars.dayCount));

    vars.spotHandle.linkTo(ext::make_shared<SimpleQuote>(8710.0));
}

void checkQuantoCorrection(bool includeDividend, bool bumpData = false) {
    const Real tolerance = 1.0e-6;

    CommonVars vars;
    ext::shared_ptr<EquityIndex> equityIndex =
        includeDividend ?
            vars.equityIndex :
            vars.equityIndex->clone(vars.localCcyInterestHandle, Handle<YieldTermStructure>(),
                                    vars.spotHandle);

    auto cf = vars.createEquityQuantoCashFlow(equityIndex);

    if (bumpData)
        bumpMarketData(vars);

    Real strike = vars.equityIndex->fixing(cf->fixingDate());
    Real indexStart = vars.equityIndex->fixing(cf->baseDate());

    Real time = vars.localCcyInterestHandle->timeFromReference(cf->fixingDate());
    Real rf = vars.localCcyInterestHandle->zeroRate(time, Continuous);
    Real q = includeDividend ? vars.dividendHandle->zeroRate(time, Continuous) : Real(0.0);
    Real eqVol = vars.equityVolHandle->blackVol(cf->fixingDate(), strike);
    Real fxVol = vars.fxVolHandle->blackVol(cf->fixingDate(), 1.0);
    Real rho = vars.correlationHandle->value();
    Real spot = vars.spotHandle->value();

    Real quantoForward = spot * std::exp((rf - q - rho * eqVol * fxVol) * time);
    Real expectedAmount = (quantoForward / indexStart - 1.0) * vars.notional;

    Real actualAmount = cf->amount();

    if ((std::fabs(actualAmount - expectedAmount) > tolerance))
        BOOST_ERROR("could not replicate equity quanto correction\n"
                    << "    actual amount:    " << actualAmount << "\n"
                    << "    expected amount:    " << expectedAmount << "\n"
                    << "    index start:    " << indexStart << "\n"
                    << "    index end:    " << quantoForward << "\n"
                    << "    local rate:    " << rf << "\n"
                    << "    equity volatility:    " << eqVol << "\n"
                    << "    FX volatility:    " << fxVol << "\n"
                    << "    correlation:    " << rho << "\n"
                    << "    spot:    " << spot << "\n");
}

void checkRaisedError(const ext::shared_ptr<EquityCashFlow>& cf, const std::string& message) {
    BOOST_CHECK_EXCEPTION(cf->amount(), Error, ExpectedErrorMessage(message));
}


BOOST_AUTO_TEST_CASE(testSimpleEquityCashFlow) {
    BOOST_TEST_MESSAGE("Testing simple equity cash flow...");

    const Real tolerance = 1.0e-6;

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow(false);

    Real indexStart = vars.equityIndex->fixing(cf->baseDate());
    Real indexEnd = vars.equityIndex->fixing(cf->fixingDate());

    Real expectedAmount = (indexEnd / indexStart - 1.0) * vars.notional;

    Real actualAmount = cf->amount();

    if ((std::fabs(actualAmount - expectedAmount) > tolerance))
        BOOST_ERROR("could not replicate simple equity quanto cash flow\n"
                    << "    actual amount:    " << actualAmount << "\n"
                    << "    expected amount:    " << expectedAmount << "\n"
                    << "    index start:    " << indexStart << "\n"
                    << "    index end:    " << indexEnd << "\n");
}

BOOST_AUTO_TEST_CASE(testQuantoCorrection) {
    BOOST_TEST_MESSAGE("Testing quanto correction...");

    checkQuantoCorrection(true);
    checkQuantoCorrection(false);

    // Checks whether observers are being notified
    // about changes in market data handles.
    checkQuantoCorrection(false, true);
}

BOOST_AUTO_TEST_CASE(testErrorWhenBaseDateAfterFixingDate) {
    BOOST_TEST_MESSAGE("Testing error when base date after fixing date...");

    CommonVars vars;

    Date end(5, January, 2023);
    Date start(5, April, 2023);

    auto cf = vars.createEquityQuantoCashFlow(vars.equityIndex, start, end);

    checkRaisedError(cf, "Fixing date cannot fall before base date.");
}

BOOST_AUTO_TEST_CASE(testErrorWhenQuantoCurveHandleIsEmpty) {
    BOOST_TEST_MESSAGE("Testing error when quanto currency curve handle is empty...");

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow();

    ext::shared_ptr<YieldTermStructure> yts;
    vars.quantoCcyInterestHandle.linkTo(yts);
    checkRaisedError(cf, "Quanto currency term structure handle cannot be empty.");
}

BOOST_AUTO_TEST_CASE(testErrorWhenEquityVolHandleIsEmpty) {
    BOOST_TEST_MESSAGE("Testing error when equity vol handle is empty...");

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow();

    ext::shared_ptr<BlackVolTermStructure> vol;
    vars.equityVolHandle.linkTo(vol);
    checkRaisedError(cf, "Equity volatility term structure handle cannot be empty.");
}

BOOST_AUTO_TEST_CASE(testErrorWhenFXVolHandleIsEmpty) {
    BOOST_TEST_MESSAGE("Testing error when FX vol handle is empty...");

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow();

    ext::shared_ptr<BlackVolTermStructure> vol;
    vars.fxVolHandle.linkTo(vol);
    checkRaisedError(cf, "FX volatility term structure handle cannot be empty.");
}

BOOST_AUTO_TEST_CASE(testErrorWhenCorrelationHandleIsEmpty) {
    BOOST_TEST_MESSAGE("Testing error when correlation handle is empty...");

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow();

    ext::shared_ptr<Quote> correlation;
    vars.correlationHandle.linkTo(correlation);
    checkRaisedError(cf, "Correlation handle cannot be empty.");
}

BOOST_AUTO_TEST_CASE(testErrorWhenInconsistentMarketDataReferenceDate) {
    BOOST_TEST_MESSAGE("Testing error when market data reference dates are inconsistent...");

    CommonVars vars;

    auto cf = vars.createEquityQuantoCashFlow();

    vars.quantoCcyInterestHandle.linkTo(flatRate(Date(26, January, 2023), 0.02, vars.dayCount));

    checkRaisedError(
        cf, "Quanto currency term structure, equity and FX volatility need to have the same "
            "reference date.");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

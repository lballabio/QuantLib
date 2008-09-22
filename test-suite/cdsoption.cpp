/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

#include "cdsoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/credit/cdsoption.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
//#include <ql/cashflows/all.hpp>
//#include <ql/indexes/all.hpp>
//#include <ql/models/shortrate/all.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CdsOptionTest::testCached() {

    BOOST_MESSAGE("Testing CDS-option value against cached values...");

    SavedSettings backup;

    Date cachedToday = Date(10,December,2007);
    Settings::instance().evaluationDate() = cachedToday;

    Calendar calendar = TARGET();

    RelinkableHandle<YieldTermStructure> riskFree;
    riskFree.linkTo(boost::shared_ptr<YieldTermStructure>(
                              new FlatForward(cachedToday,0.02,Actual360())));

    Date expiry = calendar.advance(cachedToday,9,Months);
    Date startDate = calendar.advance(expiry,1,Months);
    Date maturity = calendar.advance(startDate,7,Years);

    DayCounter dayCounter = Actual360();
    BusinessDayConvention convention = ModifiedFollowing;
    Real notional = 1000000.0;

    Handle<Quote> hazardRate(boost::shared_ptr<Quote>(new SimpleQuote(0.001)));

    Schedule schedule(startDate,maturity, Period(Quarterly),
                      calendar, convention, convention,
                      DateGeneration::Forward, false);

    Real recoveryRate = 0.4;
    Handle<DefaultProbabilityTermStructure> defaultProbability(
        boost::shared_ptr<DefaultProbabilityTermStructure>(
                                 new FlatHazardRate(hazardRate, dayCounter)));

    Issuer issuer(defaultProbability, recoveryRate);

    CreditDefaultSwap swap(Protection::Seller, notional, 0.001, schedule,
                           convention, dayCounter);
    swap.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                    new MidPointCdsEngine(issuer, riskFree)));
    Rate strike = swap.fairSpread();

    Handle<Quote> cdsVol(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));

    CdsOption option(expiry, strike, cdsVol, issuer, Protection::Seller,
                     notional, schedule, dayCounter, true, riskFree);

    Real cachedValue = 275.584421;
    if (std::fabs(option.NPV() - cachedValue) > 1.0e-5)
        BOOST_ERROR(
            "failed to reproduce cached value:\n"
            << std::fixed << std::setprecision(6)
            << "    calculated: " << option.NPV() << "\n"
            << "    expected:   " << cachedValue);
}


test_suite* CdsOptionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("range-accrual-swap tests");
    suite->add(QUANTLIB_TEST_CASE(&CdsOptionTest::testCached));
    return suite;
}

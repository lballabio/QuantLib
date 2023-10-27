/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/credit/cdsoption.hpp>
#include <ql/experimental/credit/blackcdsoptionengine.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/simplequote.hpp>

#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CdsOptionExperimentalTest)

BOOST_AUTO_TEST_CASE(testCached) {

    BOOST_TEST_MESSAGE("Testing CDS-option value against cached values...");

    Date cachedToday = Date(10,December,2007);
    Settings::instance().evaluationDate() = cachedToday;

    Calendar calendar = TARGET();

    RelinkableHandle<YieldTermStructure> riskFree;
    riskFree.linkTo(ext::shared_ptr<YieldTermStructure>(
                              new FlatForward(cachedToday,0.02,Actual360())));

    Date expiry = calendar.advance(cachedToday,9,Months);
    Date startDate = calendar.advance(expiry,1,Months);
    Date maturity = calendar.advance(startDate,7,Years);

    DayCounter dayCounter = Actual360();
    BusinessDayConvention convention = ModifiedFollowing;
    Real notional = 1000000.0;

    Handle<Quote> hazardRate(ext::shared_ptr<Quote>(new SimpleQuote(0.001)));

    Schedule schedule(startDate,maturity, Period(Quarterly),
                      calendar, convention, convention,
                      DateGeneration::Forward, false);

    Real recoveryRate = 0.4;
    Handle<DefaultProbabilityTermStructure> defaultProbability(
        ext::shared_ptr<DefaultProbabilityTermStructure>(
                    new FlatHazardRate(0, calendar, hazardRate, dayCounter)));

    ext::shared_ptr<PricingEngine> swapEngine(
           new MidPointCdsEngine(defaultProbability, recoveryRate, riskFree));

    CreditDefaultSwap swap(Protection::Seller, notional, 0.001, schedule,
                           convention, dayCounter);
    swap.setPricingEngine(swapEngine);
    Rate strike = swap.fairSpread();

    Handle<Quote> cdsVol(ext::shared_ptr<Quote>(new SimpleQuote(0.20)));

    ext::shared_ptr<CreditDefaultSwap> underlying(
         new CreditDefaultSwap(Protection::Seller, notional, strike, schedule,
                               convention, dayCounter));
    underlying->setPricingEngine(swapEngine);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(expiry));
    CdsOption option1(underlying, exercise);
    option1.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BlackCdsOptionEngine(defaultProbability, recoveryRate,
                                             riskFree, cdsVol)));

    Real cachedValue = 270.976348;
    if (std::fabs(option1.NPV() - cachedValue) > 1.0e-5)
        BOOST_ERROR("failed to reproduce cached value:\n"
                    << std::fixed << std::setprecision(6)
                    << "    calculated: " << option1.NPV() << "\n"
                    << "    expected:   " << cachedValue);

    underlying = ext::make_shared<CreditDefaultSwap>(
         Protection::Buyer, notional, strike, schedule,
                               convention, dayCounter);
    underlying->setPricingEngine(swapEngine);

    CdsOption option2(underlying, exercise);
    option2.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BlackCdsOptionEngine(defaultProbability, recoveryRate,
                                             riskFree, cdsVol)));

    cachedValue = 270.976348;
    if (std::fabs(option2.NPV() - cachedValue) > 1.0e-5)
        BOOST_ERROR("failed to reproduce cached value:\n"
                    << std::fixed << std::setprecision(6)
                    << "    calculated: " << option2.NPV() << "\n"
                    << "    expected:   " << cachedValue);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
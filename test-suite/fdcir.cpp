/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2020 Lew Wei Hao

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
#include <ql/instruments/barrieroption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdcirvanillaengine.hpp>
#include <ql/processes/coxingersollrossprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FdCIRTests)

BOOST_AUTO_TEST_CASE(testFdmCIRConvergence) {
    BOOST_TEST_MESSAGE("Testing FDM CIR convergence...");

    FdmSchemeDesc schemes[] = {
        FdmSchemeDesc::Hundsdorfer(),
        FdmSchemeDesc::ModifiedCraigSneyd(),
        FdmSchemeDesc::ModifiedHundsdorfer(),
        FdmSchemeDesc::CraigSneyd(),
        FdmSchemeDesc::TrBDF2(),
        FdmSchemeDesc::CrankNicolson(),
    };

    // set up dates
    Date today = Date::todaysDate();

    // our options
    Option::Type type(Option::Put);
    Real underlying = 36;
    Real strike = 40;
    Spread dividendYield = 0.00;
    Rate riskFreeRate = 0.06;
    Volatility volatility = 0.20;
    Date maturity = today + 365;
    DayCounter dayCounter = Actual365Fixed();

    ext::shared_ptr<Exercise> europeanExercise(
        new EuropeanExercise(maturity));

    Handle<Quote> underlyingH(
        ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

    Handle<YieldTermStructure> flatTermStructure(
        ext::shared_ptr<YieldTermStructure>(flatRate(today, riskFreeRate, dayCounter)));
    Handle<YieldTermStructure> flatDividendTS(
        ext::shared_ptr<YieldTermStructure>(flatRate(today, dividendYield, dayCounter)));
    Handle<BlackVolTermStructure> flatVolTS(
        ext::shared_ptr<BlackVolTermStructure>(flatVol(today, volatility, dayCounter)));
    ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(type, strike));
    ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
        new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                      flatTermStructure, flatVolTS));

    VanillaOption europeanOption(payoff, europeanExercise);

    Real speed = 1.2188;
    Real cirSigma = 0.02438;
    Real level = 0.0183;
    Real initialRate = 0.06;
    Real rho = 0.00789;
    Real lambda = -0.5726;
    Real newSpeed = speed + (cirSigma*lambda); //1.0792
    Real newLevel = (level * speed)/(speed + (cirSigma*lambda));//// 0.0240

    ext::shared_ptr<CoxIngersollRossProcess> cirProcess(new CoxIngersollRossProcess(newSpeed, cirSigma, initialRate, newLevel));

    Real expected = 4.275;
    Real tolerance = 0.0003;

    for (const auto& scheme : schemes) {
        ext::shared_ptr<PricingEngine> fdcirengine =
            MakeFdCIRVanillaEngine(cirProcess, bsmProcess, rho).withFdmSchemeDesc(scheme);
        europeanOption.setPricingEngine(fdcirengine);
        Real calculated = europeanOption.NPV();
        if (std::fabs(expected - calculated) > tolerance) {
            BOOST_ERROR("Failed to reproduce expected npv"
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected
                            << "\n    tolerance:  " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()


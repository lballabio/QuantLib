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

#include "varianceoption.hpp"
#include "utilities.hpp"
#include <ql/experimental/varianceoption/varianceoption.hpp>
#include <ql/experimental/varianceoption/integralhestonvarianceoptionengine.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void VarianceOptionTest::testIntegralHeston() {

    BOOST_TEST_MESSAGE("Testing variance option with integral Heston engine...");

    DayCounter dc = Actual360();
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> s0(std::make_shared<SimpleQuote>(1.0));
    Handle<YieldTermStructure> qTS;
    std::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    Real v0 = 2.0;
    Real kappa = 2.0;
    Real theta = 0.01;
    Real sigma = 0.1;
    Real rho = -0.5;

    std::shared_ptr<HestonProcess> process(new HestonProcess(rTS, qTS, s0,
                                                               v0, kappa, theta,
                                                               sigma, rho));
    std::shared_ptr<PricingEngine> engine(
                               new IntegralHestonVarianceOptionEngine(process));

    Real strike = 0.05;
    Real nominal = 1.0;
    Time T = 1.5;
    Date exDate = today + int(360*T);

    std::shared_ptr<Payoff> payoff(new PlainVanillaPayoff(Option::Call,
                                                            strike));

    VarianceOption varianceOption1(payoff, nominal, today, exDate);
    varianceOption1.setPricingEngine(engine);

    Real calculated = varianceOption1.NPV();
    Real expected = 0.9104619;
    Real error = std::fabs(calculated-expected);
    if (error>1.0e-7) {
        BOOST_ERROR(
                 "Failed to reproduce variance-option price:"
                 << "\n    expected:   " << std::setprecision(7) << expected
                 << "\n    calculated: " << std::setprecision(7) << calculated
                 << "\n    error:      " << error);
    }


    v0 = 1.5;
    kappa = 2.0;
    theta = 0.01;
    sigma = 0.1;
    rho = -0.5;

    process = std::make_shared<HestonProcess>(
               rTS, qTS, s0, v0, kappa, theta, sigma, rho);
    engine = std::shared_ptr<PricingEngine>(
                               new IntegralHestonVarianceOptionEngine(process));

    strike = 0.7;
    nominal = 1.0;
    T = 1.0;
    exDate = today + int(360*T);

    payoff = std::shared_ptr<Payoff>(new PlainVanillaPayoff(Option::Put,
                                                              strike));

    VarianceOption varianceOption2(payoff, nominal, today, exDate);
    varianceOption2.setPricingEngine(engine);

    calculated = varianceOption2.NPV();
    expected = 0.0466796;
    error = std::fabs(calculated-expected);
    if (error>1.0e-7) {
        BOOST_ERROR(
                 "Failed to reproduce variance-option price:"
                 << "\n    expected:   " << std::setprecision(7) << expected
                 << "\n    calculated: " << std::setprecision(7) << calculated
                 << "\n    error:      " << error);
    }

}

test_suite* VarianceOptionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Variance option tests");

    suite->add(QUANTLIB_TEST_CASE(&VarianceOptionTest::testIntegralHeston));
    return suite;
}


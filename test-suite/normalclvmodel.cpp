/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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


#include "utilities.hpp"
#include "normalclvmodel.hpp"

#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>

#include <ql/experimental/models/normalclvmodel.hpp>
#include <ql/experimental/finitedifferences/bsmrndcalculator.hpp>
#include <ql/experimental/finitedifferences/hestonrndcalculator.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#include <iostream>

void NormalCLVModelTest::testBSCumlativeDistributionFunction() {
    BOOST_TEST_MESSAGE("Testing Black-Scholes cumulative distribution function"
                       " with constant volatility...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(22, June, 2016);
    const Date maturity = today + Period(6, Months);

    const Real s0 = 100;
    const Real rRate = 0.1;
    const Real qRate = 0.05;
    const Volatility vol = 0.25;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));
    const Handle<BlackVolTermStructure> volTS(flatVol(today, vol, dc));

    const boost::shared_ptr<GeneralizedBlackScholesProcess> process(
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, volTS));

    const NormalCLVMCModel m(process, std::vector<Date>());
    const BSMRNDCalculator rndCalculator(process);


    const Real tol = 1e5*QL_EPSILON;
    const Time t = dc.yearFraction(today, maturity);
    for (Real x=10; x < 400; x+=10) {
        const Real calculated = m.F(maturity, x);
        const Real expected = rndCalculator.cdf(std::log(x), t);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("Failed to reproduce CDF for "
                       << "\n    strike:     " << x
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
        }
    }
}

void NormalCLVModelTest::testHestonCumlativeDistributionFunction() {
    BOOST_TEST_MESSAGE("Testing Heston cumulative distribution function ...");

    SavedSettings backup;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(22, June, 2016);
    const Date maturity = today + Period(1, Years);

    const Real s0 = 100;
    const Real v0 = 0.01;
    const Real rRate = 0.1;
    const Real qRate = 0.05;
    const Real kappa = 2.0;
    const Real theta = 0.09;
    const Real sigma = 0.4;
    const Real rho = -0.75;

    const Handle<Quote> spot(boost::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(today, qRate, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, rRate, dc));

    const boost::shared_ptr<HestonProcess> process(
        boost::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Handle<BlackVolTermStructure> hestonVolTS(
        boost::make_shared<HestonBlackVolSurface>(
            Handle<HestonModel>(boost::make_shared<HestonModel>(process))));

    const NormalCLVMCModel m(
        boost::make_shared<GeneralizedBlackScholesProcess>(
            spot, qTS, rTS, hestonVolTS),
        std::vector<Date>());

    const HestonRNDCalculator rndCalculator(process);

    const Real tol = 1e-6;
    const Time t = dc.yearFraction(today, maturity);
    for (Real x=10; x < 400; x+=25) {
        const Real calculated = m.F(maturity, x);
        const Real expected = rndCalculator.cdf(std::log(x), t);

        if (std::fabs(calculated - expected) > tol) {
            BOOST_FAIL("Failed to reproduce CDF for "
                       << "\n    strike:     " << x
                       << "\n    calculated: " << calculated
                       << "\n    expected:   " << expected);
        }
    }

}

test_suite* NormalCLVModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("NormalCLVModel tests");
    suite->add(QUANTLIB_TEST_CASE(
        &NormalCLVModelTest::testBSCumlativeDistributionFunction));
    suite->add(QUANTLIB_TEST_CASE(
        &NormalCLVModelTest::testHestonCumlativeDistributionFunction));

    return suite;
}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Skandinaviska Enskilda Banken AB (publ)

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
#include <ql/experimental/volatility/svismilesection.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SviVolatilityTests)

BOOST_AUTO_TEST_CASE(testSviSmileSection) {

    BOOST_TEST_MESSAGE("Testing SviSmileSection construction...");

    Date today = Settings::instance().evaluationDate();

    // Test time based constructor
    Time tte = 11.0 / 365;
    Real forward = 123.45;
    Real a = -0.0666;
    Real b = 0.229;
    Real sigma = 0.337;
    Real rho = 0.439;
    Real m = 0.193;
    std::vector<Real> sviParameters = {a, b, sigma, rho, m};
    // Compute the strike that yields x (log-moneyness) equal to m,
    // this simplifies the variance expression to a+b*sigma so we can test the correctness
    // against the input parameters
    Real strike = forward * std::exp(m);
    ext::shared_ptr<SviSmileSection> time_section;

    BOOST_CHECK_NO_THROW(time_section =
                             ext::make_shared<SviSmileSection>(tte, forward, sviParameters));
    BOOST_CHECK_EQUAL(time_section->atmLevel(), forward);
    QL_CHECK_CLOSE(time_section->variance(strike), a + b * sigma, 1E-10);

    // Test date based constructor
    Date date = today + Period(11, Days);
    ext::shared_ptr<SviSmileSection> date_section;

    BOOST_CHECK_NO_THROW(date_section =
                             ext::make_shared<SviSmileSection>(date, forward, sviParameters));

    BOOST_CHECK_EQUAL(date_section->atmLevel(), forward);
    QL_CHECK_CLOSE(date_section->variance(strike), a + b * sigma, 1E-10);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

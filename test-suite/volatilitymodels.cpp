/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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
#include <ql/volatilitymodel.hpp>
#include <ql/models/volatility/constantestimator.hpp>
#include <ql/models/volatility/simplelocalestimator.hpp>
#include <ql/models/volatility/garmanklass.hpp>
#include <ql/timeseries.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(VolatilityModelsTest)

BOOST_AUTO_TEST_CASE(testConstruction) {

    BOOST_TEST_MESSAGE("Testing volatility model construction...");

    TimeSeries<Real> ts;
    ts[Date(25, March, 2005)] = 1.2;
    ts[Date(29, March, 2005)] = 2.3;
    ts[Date(15, March, 2005)] = 0.3;

    SimpleLocalEstimator sle(1/360.0);
    TimeSeries<Volatility> locale = sle.calculate(ts);

    ConstantEstimator ce(1);
    TimeSeries<Volatility> sv = ce.calculate(locale);
    sv.begin();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
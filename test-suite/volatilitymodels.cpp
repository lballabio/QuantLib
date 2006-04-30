/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "volatilitymodels.hpp"
#include "utilities.hpp"
#include <ql/volatilitymodel.hpp>
#include <ql/VolatilityModels/constantestimator.hpp>
#include <ql/VolatilityModels/simplelocalestimator.hpp>
#include <ql/timeseries.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void VolatilityModelsTest::testConstruction() {

    BOOST_MESSAGE("Testing volatility model construction...");

    QL_TEST_BEGIN

    TimeSeries<Real> ts;
    ts.insert(Date(25, March, 2005), 1.2);
    ts.insert(Date(29, March, 2005), 2.3);
    ts.insert(Date(15, March, 2005), 0.3);

    SimpleLocalEstimator sle;
    TimeSeries<Volatility> locale(sle.calculate(ts));

    ConstantEstimator ce(1, 1.0/360.0);
    TimeSeries<Volatility> sv(ce.calculate(locale));
    TimeSeries<Volatility>::const_valid_iterator cur = sv.vbegin();

    QL_TEST_END
}

test_suite* VolatilityModelsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("volatility models tests");
    suite->add(BOOST_TEST_CASE(&VolatilityModelsTest::testConstruction));
    return suite;
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include "sampledcurve.hpp"
#include "utilities.hpp"
#include <ql/Math/sampledcurve.hpp>
#include <ql/Utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

class FSquared : std::unary_function<Real,Real> {
public:
    Real operator()(Real x) { return x*x;};
};

void SampledCurveTest::testConstruction() {

    BOOST_MESSAGE("Testing sampled curve construction...");

    QL_TEST_BEGIN
    SampledCurve curve(100);
    FSquared f2;
    curve.setLinearSpacing(-10.0,10.0);
    curve.sample(f2);
    Real expected = 100.0;
    if (std::fabs(curve.value(0) - expected) > 1e-5) {
        BOOST_ERROR("function sampling failed");
    }

    curve.value(0) = 2.0;
    if (std::fabs(curve.value(0) - 2.0) > 1e-5) {
        BOOST_ERROR("curve value setting failed");
    }

    Array& grid = curve.values();
    grid[1] = 3.0;
    if (std::fabs(curve.value(1) - 3.0) > 1e-5) {
        BOOST_ERROR("curve value grid failed");
    }


    QL_TEST_END
}

test_suite* SampledCurveTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("sampled curve tests");
    suite->add(BOOST_TEST_CASE(&SampledCurveTest::testConstruction));
    return suite;
}


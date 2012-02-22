/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include "surface.hpp"
#include "utilities.hpp"
#include <ql/math/surface.hpp>
#include <ql/math/domain.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void SurfaceTest::testOutput() {

    BOOST_MESSAGE("Testing surface...");
    Real tolerance = 1e-5;

    TestSurface st;
    Real out = st(0, 0);
    Real expected = 0.0;
    if (std::fabs(out - expected) > tolerance)
        BOOST_FAIL("test surface incorrect value");

    RectangularDomain rect(0.0, 0.0, 1.0, 1.0);
    if (!rect.includes(0.5, 0.5))
        BOOST_FAIL("RectangularDomain fails 0.5 0.5");
    if (rect.includes(1.5, 0.5))
        BOOST_FAIL("RectangularDomain fails 1.5 0.5");
}


test_suite* SurfaceTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Surface tests");

    suite->add(QUANTLIB_TEST_CASE(&SurfaceTest::testOutput));
    return suite;
}


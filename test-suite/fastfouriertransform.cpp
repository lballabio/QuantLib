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

#include "fastfouriertransform.hpp"
#include "utilities.hpp"
#include <ql/math/fastfouriertransform.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void FastFourierTransformTest::testConstruction() {
    BOOST_MESSAGE("Testing fast fourier transform construction...");
    QL_TEST_BEGIN
    QL_TEST_END
}

test_suite* FastFourierTransformTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("fast fourier transform tests");
    suite->add(QUANTLIB_TEST_CASE(&FastFourierTransformTest::testConstruction));
    return suite;
}


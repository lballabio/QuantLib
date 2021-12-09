/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Aprexo Limited

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

#include "compiledboostversion.hpp"
#include "utilities.hpp"

#include <ql/version.hpp>

#include <boost/version.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CompiledBoostVersionTest::test() {

    BOOST_TEST_MESSAGE("Testing compiled boost version...");

    // this will fail if the test suite is being built with a different boost version than the library was
    BOOST_CHECK(QuantLib::compiledBoostVersion() == BOOST_VERSION);
}


test_suite* CompiledBoostVersionTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Compiled boost version test");
    suite->add(QUANTLIB_TEST_CASE(&CompiledBoostVersionTest::test));
    return suite;
}


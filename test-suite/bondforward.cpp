/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2022 Marcin Rybacki

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

#include "bondforward.hpp"
#include "utilities.hpp"

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace bond_forward_test {}

void BondForwardTest::test() {
    BOOST_TEST_MESSAGE("");

    using namespace bond_forward_test;
}

test_suite* BondForwardTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Bond forward tests");

    suite->add(QUANTLIB_TEST_CASE(&BondForwardTest::test));
    return suite;
}
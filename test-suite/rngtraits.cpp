/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl
 Copyright (C) 2004 Walter Penschke

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

#include "rngtraits.hpp"
#include "utilities.hpp"
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void RngTraitsTest::testGaussian() {

    BOOST_TEST_MESSAGE("Testing Gaussian pseudo-random number generation...");

    PseudoRandom::rsg_type rsg =
        PseudoRandom::make_sequence_generator(100, 1234);

    const std::vector<Real>& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (double value : values)
        sum += value;

    Real stored = 4.09916;
    Real tolerance = 1.0e-5;
    if (std::fabs(sum - stored) > tolerance)
        BOOST_FAIL("the sum of the samples does not match the stored value\n"
                   << "    calculated: " << sum << "\n"
                   << "    expected:   " << stored);
}


void RngTraitsTest::testDefaultPoisson() {

    BOOST_TEST_MESSAGE("Testing Poisson pseudo-random number generation...");

    PoissonPseudoRandom::icInstance =
        ext::shared_ptr<InverseCumulativePoisson>();
    PoissonPseudoRandom::rsg_type rsg =
        PoissonPseudoRandom::make_sequence_generator(100, 1234);

    const std::vector<Real>& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (double value : values)
        sum += value;

    Real stored = 108.0;
    if (!close(sum, stored))
        BOOST_FAIL("the sum of the samples does not match the stored value\n"
                   << "    calculated: " << sum << "\n"
                   << "    expected:   " << stored);
}


void RngTraitsTest::testCustomPoisson() {

    BOOST_TEST_MESSAGE("Testing custom Poisson pseudo-random number generation...");

    PoissonPseudoRandom::icInstance =
        ext::make_shared<InverseCumulativePoisson>(
                                           4.0);
    PoissonPseudoRandom::rsg_type rsg =
        PoissonPseudoRandom::make_sequence_generator(100, 1234);

    const std::vector<Real>& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (double value : values)
        sum += value;

    Real stored = 409.0;
    if (!close(sum, stored))
        BOOST_FAIL("the sum of the samples does not match the stored value\n"
                   << "    calculated: " << sum << "\n"
                   << "    expected:   " << stored);
}


test_suite* RngTraitsTest::suite() {
    auto* suite = BOOST_TEST_SUITE("RNG traits tests");
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testGaussian));
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testDefaultPoisson));
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testCustomPoisson));
    return suite;
}


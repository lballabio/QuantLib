
/*
 Copyright (C) 2004 StatPro Italia srl
 Copyright (C) 2004 Walter Penschke

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "rngtraits.hpp"
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/Math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void RngTraitsTest::testGaussian() {

    BOOST_MESSAGE("Testing Gaussian pseudo-random number generation...");

    PseudoRandom::rsg_type rsg =
        PseudoRandom::make_sequence_generator(100, 1234);

    const Array& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (Size i=0; i<values.size(); i++)
        sum += values[i];

    Real stored = 4.09916;
    Real tolerance = 1.0e-5;
    if (std::fabs(sum - stored) > tolerance)
        BOOST_FAIL(
            "the sum of the samples does not match the stored value\n"
            "    calculated: " + DecimalFormatter::toString(sum) + "\n"
            "    expected:   " + DecimalFormatter::toString(stored));
}


void RngTraitsTest::testDefaultPoisson() {

    BOOST_MESSAGE("Testing Poisson pseudo-random number generation...");

    PoissonPseudoRandom::icInstance =
        boost::shared_ptr<InverseCumulativePoisson>();
    PoissonPseudoRandom::rsg_type rsg =
        PoissonPseudoRandom::make_sequence_generator(100, 1234);

    const Array& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (Size i=0; i<values.size(); i++)
        sum += values[i];

    Real stored = 108.0;
    if (!close(sum, stored))
        BOOST_FAIL(
            "the sum of the samples does not match the stored value\n"
            "    calculated: " + DecimalFormatter::toString(sum) + "\n"
            "    expected:   " + DecimalFormatter::toString(stored));
}


void RngTraitsTest::testCustomPoisson() {

    BOOST_MESSAGE("Testing custom Poisson pseudo-random number generation...");

    PoissonPseudoRandom::icInstance =
        boost::shared_ptr<InverseCumulativePoisson>(
                                           new InverseCumulativePoisson(4.0));
    PoissonPseudoRandom::rsg_type rsg =
        PoissonPseudoRandom::make_sequence_generator(100, 1234);

    const Array& values = rsg.nextSequence().value;
    Real sum = 0.0;
    for (Size i=0; i<values.size(); i++)
        sum += values[i];

    Real stored = 409.0;
    if (!close(sum, stored))
        BOOST_FAIL(
            "the sum of the samples does not match the stored value\n"
            "    calculated: " + DecimalFormatter::toString(sum) + "\n"
            "    expected:   " + DecimalFormatter::toString(stored));
}


test_suite* RngTraitsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("RNG traits tests");
    suite->add(BOOST_TEST_CASE(&RngTraitsTest::testGaussian));
    suite->add(BOOST_TEST_CASE(&RngTraitsTest::testDefaultPoisson));
    suite->add(BOOST_TEST_CASE(&RngTraitsTest::testCustomPoisson));
    return suite;
}


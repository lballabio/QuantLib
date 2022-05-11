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
#include <ql/math/randomnumbers/ranluxuniformrng.hpp>
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

#include <iostream>
void RngTraitsTest::testRanLux() {
    BOOST_TEST_MESSAGE("Testing known RanLux sequence...");

    Ranlux3UniformRng ranlux3(2938723U);
    Ranlux4UniformRng ranlux4(4390109U);

    const Real ranlux3_expected[] = {
        0.307448851544538826, 0.666313657894363587, 0.698528013702823358,
        0.0217381272445322793,0.862964516238161394, 0.909193419106014034,
        0.674484308686746914, 0.849607570377191479, 0.054626078713596371,
        0.416474163715683687
    };

    const Real ranlux4_expected[] = {
        0.222209169374078641, 0.420181950405986271, 0.0302156663005135329,
        0.0836259809475237148,0.480549766594993599, 0.723472021829124401,
        0.905819507194266293,  0.54072519936540786, 0.445908421479817463,
        0.651084788437518824
    };

    for (Size i=0; i < 10010; ++i) {
        ranlux3.next();
        ranlux4.next();
    }

    for (Size i =0; i < 10; ++i) {
        if (!close_enough(ranlux3.next().value, ranlux3_expected[i]))
            BOOST_FAIL("failed to reproduce ranlux3 numbers...");

        if (!close_enough(ranlux4.next().value, ranlux4_expected[i]))
            BOOST_FAIL("failed to reproduce ranlux4 numbers...");
    }
}

test_suite* RngTraitsTest::suite() {
    auto* suite = BOOST_TEST_SUITE("RNG traits tests");
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testGaussian));
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testDefaultPoisson));
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testCustomPoisson));
    suite->add(QUANTLIB_TEST_CASE(&RngTraitsTest::testRanLux));
    return suite;
}


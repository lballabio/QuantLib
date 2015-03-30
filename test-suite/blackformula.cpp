/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Gary Kennedy
 Copyright (C) 2015 Peter Caspers

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


#include "blackformula.hpp"
#include "utilities.hpp"
#include <ql/pricingengines/blackformula.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


void BlackFormulaTest::testBachelierImpliedVol(){


    BOOST_TEST_MESSAGE("Testing Bachelier implied vol...");

    Real forward = 1.0;
    Real bpvol = 0.01;
    Real tte = 10.0;
    Real stdDev = bpvol*std::sqrt(tte);
    Option::Type optionType = Option::Call;
    Real discount = 0.95;

    Real d[] = {-3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0};
    for(Size i=0;i<LENGTH(d);++i){


        Real strike = forward - d[i] * bpvol * std::sqrt(tte);

        Real callPrem = bachelierBlackFormula(optionType, strike, forward, stdDev, discount);

        Real impliedBpVol = bachelierBlackFormulaImpliedVol(optionType,strike, forward, tte, callPrem, discount);

        if (std::fabs(bpvol-impliedBpVol)>1.0e-12){
            BOOST_ERROR("Failed, expected " << bpvol << " realised " << impliedBpVol );
        }
    }
    return;
}

void BlackFormulaTest::testChambersImpliedVol() {

    BOOST_TEST_MESSAGE("Testing Chambers-Nawalkha implied vol approximation...");

    Option::Type types[] = {Option::Call, Option::Put};
    Real displacements[] = {0.0000, 0.0010, 0.0050, 0.0100, 0.0200};
    Real forwards[] = {-0.0010, 0.0000, 0.0050, 0.0100, 0.0200, 0.0500};
    Real strikes[] = {-0.0100, -0.0050, -0.0010, 0.0000, 0.0010, 0.0050,
                      0.0100,  0.0200,  0.0500,  0.1000};
    Real stdDevs[] = {0.10, 0.15, 0.20, 0.30, 0.50, 0.60, 0.70,
                      0.80, 1.00, 1.50, 2.00};
    Real discounts[] = {1.00, 0.95, 0.80, 1.10};

    Real tol = 5.0E-4;

    for (Size i1 = 0; i1 < LENGTH(types); ++i1) {
        for (Size i2 = 0; i2 < LENGTH(displacements); ++i2) {
            for (Size i3 = 0; i3 < LENGTH(forwards); ++i3) {
                for (Size i4 = 0; i4 < LENGTH(strikes); ++i4) {
                    for (Size i5 = 0; i5 < LENGTH(stdDevs); ++i5) {
                        for (Size i6 = 0; i6 < LENGTH(discounts); ++i6) {
                            if (forwards[i3] + displacements[i2] > 0.0 &&
                                strikes[i4] + displacements[i2] > 0.0) {
                                Real premium = blackFormula(
                                    types[i1], strikes[i4], forwards[i3],
                                    stdDevs[i5], discounts[i6],
                                    displacements[i2]);
                                Real atmPremium = blackFormula(
                                    types[i1], forwards[i3], forwards[i3],
                                    stdDevs[i5], discounts[i6],
                                    displacements[i2]);
                                Real iStdDev =
                                    blackFormulaImpliedStdDevChambers(
                                        types[i1], strikes[i4], forwards[i3],
                                        premium, atmPremium, discounts[i6],
                                        displacements[i2]);
                                Real moneyness = (strikes[i4] + displacements[i2]) /
                                             (forwards[i3] + displacements[i2]);
                                if(moneyness > 1.0) moneyness = 1.0 / moneyness;
                                Real error = (iStdDev - stdDevs[i5]) / stdDevs[i5] * moneyness;
                                if(error > tol)
                                    BOOST_ERROR(
                                        "Failed to verify Chambers-Nawalkha "
                                        "approximation for "
                                        << types[i1]
                                        << " displacement=" << displacements[i2]
                                        << " forward=" << forwards[i3]
                                        << " strike=" << strikes[i4]
                                        << " discount=" << discounts[i6]
                                        << " stddev=" << stdDevs[i5]
                                        << " result=" << iStdDev
                                        << " exceeds maximum error tolerance");
                            }
                        }
                    }
                }
            }
        }
    }
}

test_suite* BlackFormulaTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Black formula tests");

    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testBachelierImpliedVol));
    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testChambersImpliedVol));

    return suite;
}

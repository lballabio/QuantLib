
/*
 Copyright (C) 2013 Gary Kennedy

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

test_suite* BlackFormulaTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Black formula tests");

    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testBachelierImpliedVol));

    return suite;
}


/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// $Id$

#include "lowdiscrepancysequences.hpp"
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

using namespace QuantLib;
using QuantLib::RandomNumbers::SobolRsg;

CppUnit::Test* LDSTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Low discerpancy sequences' tests");
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing primitive polynomials modulo two",
                    &LDSTest::testPolynomialsModuloTwo));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Extracting Sobol Sequences",
                    &LDSTest::testSobol));
    return tests;
}


void LDSTest::testPolynomialsModuloTwo() {

    static const unsigned long jj[] = {
                 1,       1,       2,       2,       6,       6,      18,
                16,      48,      60,     176,     144,     630,     756,    
              1800,    2048,    7710,    7776,   27594,   24000,   84672,  
            120032,  356960,  276480, 1296000, 1719900, 4202496
    };

    unsigned long i=0,j=0,n=0;
    long polynomial=0;
    while (n<PPMT_MAX_DIM || polynomial!=-1) {
        if (polynomial==-1) {
            ++i; // Increase degree index
            j=0; // Reset index of polynomial in degree.
        }
        polynomial = PrimitivePolynomials[i][j];
        if (polynomial==-1) {
            --n;
            if (j!=jj[i]) {
                CPPUNIT_FAIL("Only " + IntegerFormatter::toString(j)
                    + " polynomials in degree "
                    + IntegerFormatter::toString(i+1) + " instead of "
                    + IntegerFormatter::toString(jj[i]));
            }
        }
        ++j; // Increase index of polynomial in degree i+1
        ++n; // Increase overall polynomial counter
    }

}

void LDSTest::testSobol() {

    // dummy test for the time being

    Array point;
    Size dimensionality = PPMT_MAX_DIM;
    unsigned long seed = 123456;
    SobolRsg rsg(dimensionality, seed);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
    }

    dimensionality = 100;
    seed = 123456;
    rsg = SobolRsg(dimensionality, seed);
    points = 10000;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
    }

}


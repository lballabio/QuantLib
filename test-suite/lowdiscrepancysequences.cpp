
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
using namespace QuantLib::RandomNumbers;

CppUnit::Test* LDSTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Low discerpancy sequences' tests");
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing primitive polynomials modulo two",
                    &LDSTest::testPolynomialsModuloTwo));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing Sobol Sequences",
                    &LDSTest::testSobol));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing Halton Sequences",
                    &LDSTest::testHalton));
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

    Array point;
    // testing max dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    unsigned long seed = 123456;
    SobolRsg rsg(dimensionality, seed);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            CPPUNIT_FAIL("Sobol sequence generator returns "
                " a sequence of wrong dimensionality: " +
                IntegerFormatter::toString(point.size())
                + " instead of  " +
                IntegerFormatter::toString(PPMT_MAX_DIM));
        }
    }

    // generic extractions
    dimensionality = 33;
    seed = 123456;
    rsg = SobolRsg(dimensionality, seed);
    points = 1000000;
    point = rsg.nextSequence().value;
    if (point.size()!=dimensionality) {
        CPPUNIT_FAIL("Sobol sequence generator returns "
            " a sequence of wrong dimensionality: " +
            IntegerFormatter::toString(point.size())
            + " instead of  " +
            IntegerFormatter::toString(PPMT_MAX_DIM));
    }
    for (i=1; i<points; i++) {
        point = rsg.nextSequence().value;
    }

    static const double vanderCorputSequenceModuloTwo[] = {
       // first cycle (zero excluded)
//     0.50000,
       // second cycle
       0.75000, 0.25000,
       // third cycle
       0.37500, 0.87500, 0.62500, 0.12500,
       // fourth cycle
       0.18750, 0.68750, 0.93750, 0.43750, 0.31250, 0.81250, 0.56250, 0.06250,
       // fifth cycle
       0.09375, 0.59375, 0.84375, 0.34375, 0.46875, 0.96875, 0.71875, 0.21875,
       0.15625, 0.65625, 0.90625, 0.40625, 0.28125, 0.78125, 0.53125, 0.03125
    };
    
    dimensionality = 1;
    rsg = SobolRsg(dimensionality);
    points = QL_POW(2, 5)-1-1;
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL("Sobol sequence " +
                IntegerFormatter::toString(i+1) +
                "-th number (" +
                DoubleFormatter::toString(point[0]) +
                ") is not in the van der Corput sequence modulo two: " +
                "it should have been " + 
                DoubleFormatter::toString(vanderCorputSequenceModuloTwo[i]));
        }
    }
}


void LDSTest::testHalton() {

    Array point;
    // testing "high" dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    unsigned long seed = 123456;
    HaltonRsg rsg(dimensionality);
    Size points = 100, i;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            CPPUNIT_FAIL("Sobol sequence generator returns "
                " a sequence of wrong dimensionality: " +
                IntegerFormatter::toString(point.size())
                + " instead of  " +
                IntegerFormatter::toString(PPMT_MAX_DIM));
        }
    }

    // generic extractions
    dimensionality = 33;
    seed = 123456;
    rsg = HaltonRsg(dimensionality);
    points = 1000000;

    point = rsg.nextSequence().value;
    if (point.size()!=dimensionality) {
        CPPUNIT_FAIL("Sobol sequence generator returns "
            " a sequence of wrong dimensionality: " +
            IntegerFormatter::toString(point.size())
            + " instead of  " +
            IntegerFormatter::toString(PPMT_MAX_DIM));
    }
    for (i=1; i<points; i++) {
        point = rsg.nextSequence().value;
    }

    static const double vanderCorputSequenceModuloTwo[] = {
       // first cycle (zero excluded)
       0.50000,
       // second cycle
       0.25000, 0.75000,
       // third cycle
       0.12500, 0.62500, 0.37500, 0.87500,
       // fourth cycle
       0.06250, 0.56250, 0.31250, 0.81250, 0.18750, 0.68750, 0.43750, 0.93750,
       // fifth cycle
       0.03125, 0.53125, 0.28125, 0.78125, 0.15625, 0.65625, 0.40625, 0.90625,
       0.09375, 0.59375, 0.34375, 0.84375, 0.21875, 0.71875, 0.46875, 0.96875,
    };
    

    dimensionality = 1;
    rsg = HaltonRsg(dimensionality);
    points = QL_POW(2, 5)-1;
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL("Halton sequence "+
                IntegerFormatter::toString(i+1) +
                "-th number (" +
                DoubleFormatter::toString(point[0]) +
                ") is not in the van der Corput sequence modulo two: " +
                "it should have been " + 
                DoubleFormatter::toString(vanderCorputSequenceModuloTwo[i]));
        }
    }

    static const double vanderCorputSequenceModuloThree[] = {
        // first cycle (zero excluded)
        0.333333, 0.666667,
        // second cycle
        0.111111, 0.444444, 0.777778, 0.2222220, 0.555556, 0.888889,
        // third cycle
        0.037037, 0.370370, 0.703704, 0.1481480, 0.481481, 0.814815,
        0.259259, 0.592593, 0.925926, 0.0740741, 0.407407, 0.740741,
        0.185185, 0.518519, 0.851852, 0.2962960, 0.629630, 0.962963
    };


    dimensionality = 2;
    rsg = HaltonRsg(dimensionality);
    points = QL_POW(3, 3)-1;
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL("Halton sequence dimension two first component " +
                IntegerFormatter::toString(i+1) +
                "-th number (" +
                DoubleFormatter::toString(point[0]) +
                ") is not in the van der Corput sequence modulo two: " +
                "it should have been " + 
                DoubleFormatter::toString(vanderCorputSequenceModuloTwo[i]));
        }
        if (fabs(point[1]-vanderCorputSequenceModuloThree[i])>1e-6) {
            CPPUNIT_FAIL("Halton sequence dimension two second component " +
                IntegerFormatter::toString(i+1) +
                "-th number (" +
                DoubleFormatter::toString(point[1]) +
                ") is not in the van der Corput sequence modulo three: "
                "it should have been " + 
                DoubleFormatter::toString(vanderCorputSequenceModuloThree[i]));

        }
    }

}


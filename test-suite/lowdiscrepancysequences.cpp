
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
using namespace QuantLib::Math;
using namespace QuantLib::RandomNumbers;
using namespace QuantLib::Math;

CppUnit::Test* LDSTest::suite() {
    CppUnit::TestSuite* tests =
        new CppUnit::TestSuite("Low discrepancy sequences' tests");
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing primitive polynomials modulo two",
                    &LDSTest::testPolynomialsModuloTwo));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing Sobol sequences",
                    &LDSTest::testSobol));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing Halton sequences",
                    &LDSTest::testHalton));
    tests->addTest(new CppUnit::TestCaller<LDSTest>
                   ("Testing sequences' discrepancy",
                    &LDSTest::testDiscrepancy));
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
                         IntegerFormatter::toString(dimensionality));
        }
    }

    // testing homogeneity properties
    dimensionality = 33;
    seed = 123456;
    rsg = SobolRsg(dimensionality, seed);
    SequenceStatistics<> stat(dimensionality);
    std::vector<double> mean, stdev, variance, skewness, kurtosis;
    Size k = 0;
    for (int j=1; j<5; j++) { // five cycle
//        std::cout << j << ", " ;
        points = Size(QL_POW(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        for (i=0; i<dimensionality; i++) {
            if (mean[i] != 0.5) {
                CPPUNIT_FAIL(IntegerFormatter::toOrdinal(i+1) +
                             " dimension mean (" +
                             DoubleFormatter::toString(mean[i]) +
                             ") at the end of the " +
                             IntegerFormatter::toOrdinal(j+1) +
                             " cycle in Sobol sequence is not " +
                             DoubleFormatter::toString(0.5));
            }
        }
    }


    // testing first dimension (van der Corput sequence)
    const double vanderCorputSequenceModuloTwo[] = {
        // first cycle (zero excluded)
        0.50000,
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
    points = Size(QL_POW(2.0, 5))-1; // five cycles
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL(IntegerFormatter::toOrdinal(i+1) +
                         " draw (" +
                         DoubleFormatter::toString(point[0]) +
                         ") in 1-D Sobol sequence is not in the "
                         "van der Corput sequence modulo two: " +
                         "it should have been " +
                         DoubleFormatter::toString(
                             vanderCorputSequenceModuloTwo[i]));
        }
    }
}


void LDSTest::testHalton() {

    Array point;
    // testing "high" dimensionality
    Size dimensionality = PPMT_MAX_DIM;
    unsigned long seed = 123456;
    HaltonRsg rsg(dimensionality);
    Size points = 100, i, k;
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point.size()!=dimensionality) {
            CPPUNIT_FAIL("Halton sequence generator returns "
                         " a sequence of wrong dimensionality: " +
                         IntegerFormatter::toString(point.size())
                         + " instead of  " +
                         IntegerFormatter::toString(dimensionality));
        }
    }


    // testing first and second dimension (van der Corput sequence)
    const double vanderCorputSequenceModuloTwo[] = {
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
    points = Size(QL_POW(2.0, 5))-1;  // five cycles
    for (i=0; i<points; i++) {
        point = rsg.nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL(IntegerFormatter::toOrdinal(i+1) +
                         " draw (" +
                         DoubleFormatter::toString(point[0]) +
                         ") in 1-D Halton sequence is not in the "
                         "van der Corput sequence modulo two: " +
                         "it should have been " +
                         DoubleFormatter::toString(
                             vanderCorputSequenceModuloTwo[i]));
        }
    }

    const double vanderCorputSequenceModuloThree[] = {
        // first cycle (zero excluded)
        1.0/3,  2.0/3,
        // second cycle
        1.0/9,  4.0/9,  7.0/9,  2.0/9,  5.0/9,  8.0/9,
        // third cycle
        1.0/27, 10.0/27, 19.0/27,  4.0/27, 13.0/27, 22.0/27,
        7.0/27, 16.0/27, 25.0/27,  2.0/27, 11.0/27, 20.0/27,
        5.0/27, 14.0/27, 23.0/27,  8.0/27, 17.0/27, 26.0/27
    };

    dimensionality = 2;
    rsg = HaltonRsg(dimensionality);
    points = Size(QL_POW(3.0, 3))-1;  // three cycles of the higher dimension
    for (i=0; i<points; i++) {
        point = rsg .nextSequence().value;
        if (point[0]!=vanderCorputSequenceModuloTwo[i]) {
            CPPUNIT_FAIL("First component of " +
                         IntegerFormatter::toOrdinal(i+1) +
                         " draw (" +
                         DoubleFormatter::toString(point[0]) +
                         ") in 2-D Halton sequence is not in the "
                         "van der Corput sequence modulo two: " +
                         "it should have been " +
                         DoubleFormatter::toString(
                             vanderCorputSequenceModuloTwo[i]));
        }
        if (QL_FABS(point[1]-vanderCorputSequenceModuloThree[i])>1.0e-15) {
            CPPUNIT_FAIL("Second component of " +
                         IntegerFormatter::toOrdinal(i+1) +
                         " draw (" +
                         DoubleFormatter::toString(point[1]) +
                         ") in 2-D Halton sequence is not in the "
                         "van der Corput sequence modulo three: "
                         "it should have been " +
                         DoubleFormatter::toString(
                             vanderCorputSequenceModuloThree[i]));
        }
    }




    // testing homogeneity properties
    dimensionality = 33;
    seed = 123456;
    rsg = HaltonRsg(dimensionality);
    SequenceStatistics<> stat(dimensionality);
    std::vector<double> mean, stdev, variance, skewness, kurtosis;
    k = 0;
    int j;
    for (j=1; j<5; j++) { // five cycle
        points = Size(QL_POW(2.0, j))-1; // base 2
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        if (mean[0] != 0.5) {
            CPPUNIT_FAIL("First dimension mean (" +
                         DoubleFormatter::toString(mean[0]) +
                         ") at the end of the " +
                         IntegerFormatter::toOrdinal(j+1) +
                         " cycle in Halton sequence is not " +
                         DoubleFormatter::toString(0.5));
        }
    }

    // reset generator and statistics
    rsg  = HaltonRsg(dimensionality);
    stat.reset(dimensionality);
    k = 0;
    for (j=1; j<3; j++) { // three cycle
        points = Size(QL_POW(3.0, j))-1; // base 3
        for (; k<points; k++) {
            point = rsg.nextSequence().value;
            stat.add(point);
        }
        mean = stat.mean();
        if (QL_FABS(mean[1]-0.5)>1e-16) {
            CPPUNIT_FAIL("Second dimension mean (" +
                         DoubleFormatter::toString(mean[1]) +
                         ") at the end of the " +
                         IntegerFormatter::toOrdinal(j+1) +
                         " cycle in Halton sequence is not " +
                         DoubleFormatter::toString(0.5));
        }
    }

}

void LDSTest::testDiscrepancy() {

    Array point;
    unsigned long dim, dimensionality[] = {2, 3, 5, 10, 15, 30, 50, 100 };
    unsigned long seed = 123456;

//    std::cout << std::endl;

    for (int i = 0; i<8; i++) {
        dim = dimensionality[i];
        MersenneTwisterUniformRsg mer(dim, seed);
        HaltonRsg                 hal(dim);
        SobolRsg                  sob(dim, seed);
        SobolRsg                  unS(dim, seed, true);

        DiscrepancyStatistics merStat(dim);
        DiscrepancyStatistics halStat(dim);
        DiscrepancyStatistics sobStat(dim);
        DiscrepancyStatistics unSStat(dim);
//        std::cout << "dim " << dim << std::endl;

        Size k = 0;
//        for (int j=10; j<17; j++) {
        for (int j=10; j<11; j++) {
            Size points = Size(QL_POW(2.0, j))-1;
            for (; k<points; k++) {
                point = mer.nextSequence().value;
                merStat.add(point);
                point = hal.nextSequence().value;
                halStat.add(point);
                point = sob.nextSequence().value;
                sobStat.add(point);
                point = unS.nextSequence().value;
                unSStat.add(point);
            }
/*
            std::cout << points << ": "
                "r " << QL_SQRT((1.0/QL_POW(2.0, int(dim))
                                 -1.0/QL_POW(3.0, int(dim)))
                                /points) <<
                ", Mers. " << merStat.discrepancy() <<
                ", Halt. " << halStat.discrepancy() <<
                ", Sobol " << sobStat.discrepancy() <<
                ", UnitS " << unSStat.discrepancy() <<
                std::endl;
*/
        }
    }

}


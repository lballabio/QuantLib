
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
#include <fstream>

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
                   ("Testing Halton and Sobol discrepancy",
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

    static const double vanderCorputSequenceModuloThree[] = {
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

    // reset generator and gaussianstatistics
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

    const double discrRandDim2[]   = { 0.0116519,  0.00823711, 0.0058238,  0.0041178,  0.00291164, 0.00205881, 0.00145578};
    const double discrRandDim3[]   = { 0.00927283, 0.00655528, 0.00463471, 0.00327704, 0.00231714, 0.00163844};
    const double discrRandDim5[]   = { 0.00515021};
    const double discrRandDim10[]  = { 0.000968531};
    const double discrRandDim15[]  = { 0.000172521};
    const double discrRandDim30[]  = { 9.54138e-007};
    const double discrRandDim50[]  = { 9.31778e-010};
    const double discrRandDim100[] = { 2.77691e-017};
    const double * const discrRand[8] = { discrRandDim2,  discrRandDim3,
        discrRandDim5,  discrRandDim10, discrRandDim15, discrRandDim30,
        discrRandDim50, discrRandDim100 };

    const double discrMersDim2[]   = { 0.00884306, 0.00542034, 0.00522887, 0.00447229, 0.00475131, 0.00310862, 0.00297139};
    const double discrMersDim3[]   = { 0.00701655, 0.00494472, 0.00482102, 0.00490607, 0.00333076, 0.00279909};
    const double discrMersDim5[]   = { 0.00428155};
    const double discrMersDim10[]  = { 0.000883079};
    const double discrMersDim15[]  = { 0.000163131};
    const double discrMersDim30[]  = { 4.38406e-007};
    const double discrMersDim50[]  = { 3.27135e-010};
    const double discrMersDim100[] = { 5.29737e-019};
    const double * const discrMers[8] = { discrMersDim2,  discrMersDim3,
        discrMersDim5,  discrMersDim10, discrMersDim15, discrMersDim30,
        discrMersDim50, discrMersDim100 };

    const double discrHaltDim2[]   = { 0.00125753, 0.000673163, 0.000334633, 0.000190806, 0.000111483, 5.05162e-005, 2.42083e-005};
    const double discrHaltDim3[]   = { 0.00162592, 0.000961606, 0.00048284,  0.000266769, 0.000140542, 7.63577e-005};
    const double discrHaltDim5[]   = { 0.00193329};
    const double discrHaltDim10[]  = { 0.00123382};
    const double discrHaltDim15[]  = { 0.000574585};
    const double discrHaltDim30[]  = { 0.000445345};
    const double discrHaltDim50[]  = { 0.000404071};
    const double discrHaltDim100[] = { 0.000362872};
    const double * const discrHalt[8] = { discrHaltDim2,  discrHaltDim3,
        discrHaltDim5,  discrHaltDim10, discrHaltDim15, discrHaltDim30,
        discrHaltDim50, discrHaltDim100 };

    const double discrSoboDim2[]   = { 0.000832648, 0.000431559, 0.000223876, 0.000111926, 5.69138e-005, 2.14101e-005}; // the last one is -1.#IND ???
    const double discrSoboDim3[]   = { 0.00120968,  0.00063685,  0.000339578, 0.000174813, 9.21381e-005, 4.79305e-005};
    const double discrSoboDim5[]   = { 0.0201095};
    const double discrSoboDim10[]  = { 0.00167361};
    const double discrSoboDim15[]  = { 0.000315283};
    const double discrSoboDim30[]  = { 7.39576e-007};
    const double discrSoboDim50[]  = { 6.85048e-010};
    const double discrSoboDim100[] = { 2.2668e-018};
    const double * const discrSobo[8] = { discrSoboDim2,  discrSoboDim3,
        discrSoboDim5,  discrSoboDim10, discrSoboDim15, discrSoboDim30,
        discrSoboDim50, discrSoboDim100 };

    const double discrUnSoDim2[]   = { 0.000832648, 0.000431559, 0.000223876, 0.000111926, 5.69138e-005, 2.14101e-005}; // the last one is -1.#IND ???
    const double discrUnSoDim3[]   = { 0.00120968,  0.00063685,  0.000339578, 0.000174813, 9.21381e-005, 4.79305e-005};
    const double discrUnSoDim5[]   = { 0.0119804};
    const double discrUnSoDim10[]  = { 0.00124387};
    const double discrUnSoDim15[]  = { 0.0001963648626249};
    const double discrUnSoDim30[]  = { 0.0000275078009842};
    const double discrUnSoDim50[]  = { 9.53743e-006};
    const double discrUnSoDim100[] = { 1.54476e-006};
    const double * const discrUnSo[8] = { discrUnSoDim2,  discrUnSoDim3,
        discrUnSoDim5,  discrUnSoDim10, discrUnSoDim15, discrUnSoDim30,
        discrUnSoDim50, discrUnSoDim100 };

    static const unsigned long dimensionality[] = {2, 3, 5, 10, 15, 30, 50, 100 };

    Array point;
    unsigned long dim;
    unsigned long seed = 123456;
    double trueRandomFactor;

    std::ofstream outStream("discrepancyDim100.txt");
    for (int i = 0; i<8; i++) {
        dim = dimensionality[i];
        trueRandomFactor = (1.0/QL_POW(2.0, int(dim))
            -1.0/QL_POW(3.0, int(dim)));
 
        outStream << "******" << dim << std::endl;
        MersenneTwisterUniformRsg mer(dim, seed);
        HaltonRsg                 hal(dim);
        SobolRsg                  sob(dim, seed);
        SobolRsg                  unS(dim, seed, true);

        DiscrepancyStatistics merStat(dim);
        DiscrepancyStatistics halStat(dim);
        DiscrepancyStatistics sobStat(dim);
        DiscrepancyStatistics unSStat(dim);

        double ranDiscr, merDiscr, halDiscr, sobDiscr, unSDiscr;
        double tolerance=1e-4;

        Size k = 0;
        Size sampleLoops, jMin = 10;
        // it would take too long for usual/frequent test running
//        sampleLoops = 7;
        sampleLoops = 1;
        for (int j=jMin; j<jMin+sampleLoops; j++) {
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

            ranDiscr = QL_SQRT(trueRandomFactor/points);
            if (QL_FABS(ranDiscr-discrRand[i][j-jMin])>tolerance*ranDiscr) {
                CPPUNIT_FAIL("True random discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(ranDiscr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrRand[i][j-jMin], 16));
            }

            merDiscr = merStat.discrepancy();
            if (QL_FABS(merDiscr-discrMers[i][j-jMin])>tolerance*merDiscr) {
                CPPUNIT_FAIL("Mersenne discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(merDiscr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrMers[i][j-jMin], 16));
            }

            halDiscr = halStat.discrepancy();
            if (QL_FABS(halDiscr-discrHalt[i][j-jMin])>tolerance*halDiscr) {
                CPPUNIT_FAIL("Halton discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(halDiscr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrHalt[i][j-jMin], 16));
            }

            sobDiscr = sobStat.discrepancy();
            if (QL_FABS(sobDiscr-discrSobo[i][j-jMin])>tolerance*sobDiscr) {
                CPPUNIT_FAIL("Sobol discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(sobDiscr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrSobo[i][j-jMin], 16));
            }

            unSDiscr = unSStat.discrepancy();
            if (QL_FABS(unSDiscr-discrUnSo[i][j-jMin])>tolerance*unSDiscr) {
                CPPUNIT_FAIL("Unit Sobol discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(unSDiscr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrUnSo[i][j-jMin], 16));
            }

            outStream << "points: " << points << std::endl;
            outStream <<
                "ra " << ranDiscr <<
                " M " << merDiscr <<
                " H " << halDiscr <<
                " S " << sobDiscr <<
                " U " << unSDiscr <<
                std::endl;
        }
    }
    outStream.close();

}


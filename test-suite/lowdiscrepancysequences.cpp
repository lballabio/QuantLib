
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

    const double discrRandDim2[]   = { 1.16519e-002, 8.23711e-003, 5.82380e-003, 4.11780e-003, 2.91164e-003, 2.05881e-003, 1.45578e-003};
    const double discrMersDim2[]   = { 8.84306e-003, 5.42034e-003, 5.22887e-003, 4.47229e-003, 4.75131e-003, 3.10862e-003, 2.97139e-003};
    const double discrHaltDim2[]   = { 1.25753e-003, 6.73163e-004, 3.34633e-004, 1.90806e-004, 1.11483e-004, 5.05162e-005, 2.42083e-005};
    const double discrSoboDim2[]   = { 8.32648e-004, 4.31559e-004, 2.23876e-004, 1.11926e-004, 5.69138e-005, 2.14101e-005}; // the last one is -1.#IND ???
    const double discrUnSoDim2[]   = { 8.32648e-004, 4.31559e-004, 2.23876e-004, 1.11926e-004, 5.69138e-005, 2.14101e-005}; // the last one is -1.#IND ???

    const double discrRandDim3[]   = { 9.27283e-003, 6.55528e-003, 4.63471e-003, 3.27704e-003, 2.31714e-003, 1.63844e-003, 1.15855e-003};
    const double discrMersDim3[]   = { 7.01655e-003, 4.94472e-003, 4.82102e-003, 4.90607e-003, 3.33076e-003, 2.79909e-003, 2.62083e-003};
    const double discrHaltDim3[]   = { 1.62592e-003, 9.61606e-004, 4.82840e-004, 2.66769e-004, 1.40542e-004, 7.63577e-005, 3.92584e-005};
    const double discrSoboDim3[]   = { 1.20968e-003, 6.36850e-004, 3.39578e-004, 1.74813e-004, 9.21381e-005, 4.79305e-005, 2.55803e-005};
    const double discrUnSoDim3[]   = { 1.20968e-003, 6.36850e-004, 3.39578e-004, 1.74813e-004, 9.21381e-005, 4.79305e-005, 2.55803e-005};

    const double discrRandDim5[]   = { 5.15021e-003, 3.64086e-003, 2.57416e-003, 1.82010e-003, 1.28696e-003, 9.10007e-004, 6.43467e-004};
    const double discrMersDim5[]   = { 4.28155e-003, 3.47563e-003, 2.47862e-003, 1.98171e-003, 1.56723e-003, 1.39142e-003, 6.33197e-004};
    const double discrHaltDim5[]   = { 1.93329e-003, 1.22508e-003, 6.89233e-004, 4.21737e-004, 2.12928e-004, 1.25177e-004, 7.17258e-005};
    const double discrSoboDim5[]   = { 2.01095e-002, 2.01790e-002, 2.02408e-002, 2.02626e-002, 2.02739e-002, 2.02811e-002, 2.02825e-002};
    const double discrUnSoDim5[]   = { 1.19804e-002, 1.20071e-002, 1.20408e-002, 1.20533e-002, 1.20640e-002, 1.20681e-002, 1.20705e-002};

    const double discrRandDim10[]  = { 9.68531e-004, 6.84688e-004, 4.84088e-004, 3.42281e-004, 2.42022e-004, 1.71133e-004, 1.21008e-004};
    const double discrMersDim10[]  = { 8.83079e-004, 6.55531e-004, 4.86688e-004, 3.36795e-004, 3.06489e-004, 1.73377e-004, 1.42887e-004};
    const double discrHaltDim10[]  = { 1.23382e-003, 6.89371e-004, 4.02918e-004, 2.82652e-004, 1.60818e-004, 1.07954e-004, 6.68686e-005};
    const double discrSoboDim10[]  = { 1.67361e-003, 1.69016e-003, 1.67837e-003, 1.66587e-003, 1.68271e-003, 1.69468e-003, 1.69607e-003};
    const double discrUnSoDim10[]  = { 1.24387e-003, 1.22087e-003, 1.17548e-003, 1.17424e-003, 1.17591e-003, 1.17819e-003, 1.17935e-003};

    const double discrRandDim15[]  = { 1.72521e-004, 1.21961e-004, 8.62287e-005, 6.09691e-005, 4.31104e-005, 3.04832e-005, 2.15547e-005};
    const double discrMersDim15[]  = { 1.63131e-004, 1.12392e-004, 8.35828e-005, 6.08550e-005, 4.33797e-005, 2.94889e-005, 2.10316e-005};
    const double discrHaltDim15[]  = { 5.74585e-004, 3.12068e-004, 1.70246e-004, 9.88729e-005, 5.33232e-005, 3.45373e-005, 2.11297e-005};
    const double discrSoboDim15[]  = { 3.15283e-004, 3.12619e-004, 2.96913e-004, 2.79193e-004, 2.89383e-004, 2.94596e-004, 2.97494e-004};
    const double discrUnSoDim15[]  = { 1.96365e-004, 1.93437e-004, 1.59220e-004, 1.39165e-004, 1.40769e-004, 1.37988e-004, 1.36549e-004};

    const double discrRandDim30[]  = { 9.54138e-007, 6.74513e-007, 4.76894e-007, 3.37194e-007, 2.38425e-007, 1.68590e-007, 1.19210e-007};
    const double discrMersDim30[]  = { 4.38406e-007, 3.25086e-007, 4.47206e-007, 2.85141e-007, 2.03432e-007, 1.50003e-007, 1.17126e-007};
    const double discrHaltDim30[]  = { 4.45345e-004, 2.22563e-004, 1.11257e-004, 5.56243e-005, 2.78125e-005, 1.39068e-005, 6.95399e-006};
    const double discrSoboDim30[]  = { 7.39576e-007, 2.26656e-006, 1.30958e-006, 8.29286e-007, 8.56462e-007, 5.43148e-007, 4.48368e-007};
    const double discrUnSoDim30[]  = { 2.75078e-005, 1.37584e-005, 6.87822e-006, 3.44616e-006, 1.76593e-006, 8.99183e-007, 5.44834e-007};

    const double discrRandDim50[]  = { 9.31778e-010, 6.58705e-010, 4.65718e-010, 3.29292e-010, 2.32838e-010, 1.64639e-010};
    const double discrMersDim50[]  = { 3.27135e-010, 2.42291e-010, 1.46748e-010, 1.97764e-010, 2.31423e-010, 1.30260e-010};
    const double discrHaltDim50[]  = { 4.04071e-004, 2.01937e-004, 1.00944e-004, 5.04658e-005, 2.52313e-005, 1.26153e-005};
    const double discrSoboDim50[]  = { 6.85048e-010, 3.66033e-009, 1.87978e-009, 1.45576e-009, 7.74236e-010, 1.08362e-009};
    const double discrUnSoDim50[]  = { 9.53743e-006, 4.76639e-006, 2.38261e-006, 1.19116e-006, 5.95544e-007, 2.97763e-007};

    const double discrRandDim100[] = { 2.77691e-017, 1.96309e-017, 1.38795e-017, 9.81368e-018, 6.93911e-018, 4.90661e-018};
    const double discrMersDim100[] = { 5.29737e-019, 7.29315e-019, 3.70998e-019, 3.33356e-019, 1.32975e-017, 6.69502e-018};
    const double discrHaltDim100[] = { 3.62872e-004, 1.81348e-004, 9.06517e-005, 4.53203e-005, 2.26588e-005, 1.13290e-005};
    const double discrSoboDim100[] = { 2.26680e-018, 1.18441e-018, 6.02094e-019, 2.06352e-018, 2.26136e-018, 3.46598e-018};
    const double discrUnSoDim100[] = { 1.54476e-006, 7.72003e-007, 3.85907e-007, 1.92930e-007, 9.64591e-008, 4.82281e-008};

    const double * const discrRand[8] = { discrRandDim2,  discrRandDim3,
        discrRandDim5,  discrRandDim10, discrRandDim15, discrRandDim30,
        discrRandDim50, discrRandDim100 };
    const double * const discrMers[8] = { discrMersDim2,  discrMersDim3,
        discrMersDim5,  discrMersDim10, discrMersDim15, discrMersDim30,
        discrMersDim50, discrMersDim100 };
    const double * const discrHalt[8] = { discrHaltDim2,  discrHaltDim3,
        discrHaltDim5,  discrHaltDim10, discrHaltDim15, discrHaltDim30,
        discrHaltDim50, discrHaltDim100 };
    const double * const discrSobo[8] = { discrSoboDim2,  discrSoboDim3,
        discrSoboDim5,  discrSoboDim10, discrSoboDim15, discrSoboDim30,
        discrSoboDim50, discrSoboDim100 };
    const double * const discrUnSo[8] = { discrUnSoDim2,  discrUnSoDim3,
        discrUnSoDim5,  discrUnSoDim10, discrUnSoDim15, discrUnSoDim30,
        discrUnSoDim50, discrUnSoDim100 };

    static const unsigned long dimensionality[] = {2, 3, 5, 10, 15, 30, 50, 100 };

    Array point;
    unsigned long dim;
    unsigned long seed = 123456;
    double trueRandomFactor, discr, tolerance=1e-4;

    std::ofstream outStream("discrepancy.txt");
    for (int i = 0; i<8; i++) {
        outStream << std::endl;
        dim = dimensionality[i];
        DiscrepancyStatistics stat(dim);

        trueRandomFactor = (1.0/QL_POW(2.0, int(dim))
            -1.0/QL_POW(3.0, int(dim)));
        MersenneTwisterUniformRsg mer(dim, seed);
        HaltonRsg                 hal(dim);
        SobolRsg                  sob(dim, seed);
        SobolRsg                  unS(dim, seed, true);

        Size j, k, sampleLoops = 7, jMin = 10;
        // 7 loops would take too long for usual/frequent test running
        sampleLoops = 1;

        // true random numbers
        k = 0;
        stat.reset();
        outStream << "discrRandDim" << dim << "[] = { " ;
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(QL_POW(2.0, j))-1;

            discr = QL_SQRT(trueRandomFactor/points);

            if (j!=jMin) outStream << ", ";
            outStream << discr;

            if (QL_FABS(discr-discrRand[i][j-jMin])>tolerance*discr) {
                CPPUNIT_FAIL("True random discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(discr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrRand[i][j-jMin], 16));
            }
        }
        outStream << "};" << std::endl;

        // Mersenne sequences
        k = 0;
        stat.reset();
        outStream << "discrMersDim" << dim << "[] = { " ;
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(QL_POW(2.0, j))-1;
            for (; k<points; k++) {
                point = mer.nextSequence().value;
                stat.add(point);
            }

            discr = stat.discrepancy();

            if (j!=jMin) outStream << ", ";
            outStream << discr;

            if (QL_FABS(discr-discrMers[i][j-jMin])>tolerance*discr) {
                CPPUNIT_FAIL("Mersenne discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(discr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrMers[i][j-jMin], 16));
            }
        }
        outStream << "};" << std::endl;

        // Halton sequences
        k = 0;
        stat.reset();
        outStream << "discrHaltDim" << dim << "[] = { " ;
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(QL_POW(2.0, j))-1;
            for (; k<points; k++) {
                point = hal.nextSequence().value;
                stat.add(point);
            }

            discr = stat.discrepancy();

            if (j!=jMin) outStream << ", ";
            outStream << discr;

            if (QL_FABS(discr-discrHalt[i][j-jMin])>tolerance*discr) {
                CPPUNIT_FAIL("Halton discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(discr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrHalt[i][j-jMin], 16));
            }
        }
        outStream << "};" << std::endl;

        // Sobol sequences
        k = 0;
        stat.reset();
        outStream << "discrSoboDim" << dim << "[] = { " ;
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(QL_POW(2.0, j))-1;
            for (; k<points; k++) {
                point = sob.nextSequence().value;
                stat.add(point);
            }

            discr = stat.discrepancy();

            if (j!=jMin) outStream << ", ";
            outStream << discr;

            if (QL_FABS(discr-discrSobo[i][j-jMin])>tolerance*discr) {
                CPPUNIT_FAIL("Sobol discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(discr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrSobo[i][j-jMin], 16));
            }

        }
        outStream << "};" << std::endl;

        // Unit Sobol sequences
        k = 0;
        stat.reset();
        outStream << "discrUnSoDim" << dim << "[] = { " ;
        for (j=jMin; j<jMin+sampleLoops; j++) {
            Size points = Size(QL_POW(2.0, j))-1;
            for (; k<points; k++) {
                point = unS.nextSequence().value;
                stat.add(point);
            }

            discr = stat.discrepancy();

            if (j!=jMin) outStream << ", ";
            outStream << discr;

            if (QL_FABS(discr-discrUnSo[i][j-jMin])>tolerance*discr) {
                CPPUNIT_FAIL("Unit Sobol discrepancy dimension " +
                             IntegerFormatter::toString(dimensionality[i]) +
                             " at " +
                             IntegerFormatter::toString(points) +
                             " samples is " +
                             DoubleFormatter::toString(discr, 16) +
                             " instead of " +
                             DoubleFormatter::toString(discrUnSo[i][j-jMin], 16));
            }

        }
        outStream << "};" << std::endl;

    }
    outStream.close();

}


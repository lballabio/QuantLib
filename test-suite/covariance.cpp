
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl

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

#include "covariance.hpp"
#include "utilities.hpp"
#include <ql/MonteCarlo/getcovariance.hpp>
#include <ql/Math/pseudosqrt.hpp>
#include <ql/Math/sequencestatistics.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CovarianceTest::testSalvagingCorrelation() {

    BOOST_MESSAGE("Testing correlation-salvaging algorithms...");

    double expected, calculated;

    Size n = 3;

    Matrix badCorr(n, n);
    badCorr[0][0] = 1.0; badCorr[0][1] = 0.9; badCorr[0][2] = 0.7;
    badCorr[1][0] = 0.9; badCorr[1][1] = 1.0; badCorr[1][2] = 0.3;
    badCorr[2][0] = 0.7; badCorr[2][1] = 0.3; badCorr[2][2] = 1.0;

    Matrix goodCorr(n, n);
    goodCorr[0][0] = goodCorr[1][1] = goodCorr[2][2] = 1.00000000000;
    goodCorr[0][1] = goodCorr[1][0] = 0.894024408508599;
    goodCorr[0][2] = goodCorr[2][0] = 0.696319066114392;
    goodCorr[1][2] = goodCorr[2][1] = 0.300969036104592;

    Matrix b = pseudoSqrt(badCorr, SalvagingAlgorithm::Spectral);
//    Matrix b = pseudoSqrt(badCorr, Hypersphere);
    Matrix calcCorr = b * transpose(b);

    for (Size i=0; i<n; i++) {
        for (Size j=0; j<n; j++) {
            expected   = goodCorr[i][j];
            calculated = calcCorr[i][j];
            if (QL_FABS(calculated-expected) > 1.0e-10)
                BOOST_FAIL("SalvagingCorrelation with spectral alg"
                           "cor[" + SizeFormatter::toString(i) + "]"
                           "[" + SizeFormatter::toString(j) + "]:\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,16) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,16));
        }
    }
}

void CovarianceTest::testCovariance() {

    BOOST_MESSAGE("Testing covariance calculation...");

    double data00[] = { 3.0,  9.0 };
    double data01[] = { 2.0,  7.0 };
    double data02[] = { 4.0, 12.0 };
    double data03[] = { 5.0, 15.0 };
    double data04[] = { 6.0, 17.0 };
    double* data[5] = { data00, data01, data02, data03, data04 };
    std::vector<double> weights(LENGTH(data), 1.0);

    Size i, j, n = LENGTH(data00);

    Matrix expCor(n, n);
    expCor[0][0] = 1.0000000000000000; expCor[0][1] = 0.9970544855015813;
    expCor[1][0] = 0.9970544855015813; expCor[1][1] = 1.0000000000000000;

    SequenceStatistics<> s(n);
    std::vector<double> temp(n);

    for (i = 0; i<LENGTH(data); i++) {
        for (j=0; j<n; j++) {
            temp[j]= data[i][j];
        }
        s.add(temp, weights[i]);
    }

    std::vector<double> m = s.mean();
    std::vector<double> std = s.standardDeviation();
    Matrix calcCov  =  s.covariance();
    Matrix calcCor  =  s.correlation();

    Matrix expCov(n, n);
    for (i=0; i<n; i++) {
        expCov[i][i] = std[i]*std[i];
        for (j=0; j<i; j++) {
            expCov[i][j] = expCov[j][i] = expCor[i][j]*std[i]*std[j];
        }
    }

    double expected, calculated;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            expected   =  expCor[i][j];
            calculated = calcCor[i][j];
            if (QL_FABS(calculated-expected) > 1.0e-10)
                BOOST_FAIL("SequenceStatistics "
                           "cor[" + SizeFormatter::toString(i) + "]"
                           "[" + SizeFormatter::toString(j) + "]:\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,16) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,16));

            expected   =  expCov[i][j];
            calculated = calcCov[i][j];
            if (QL_FABS(calculated-expected) > 1.0e-10)
                BOOST_FAIL("SequenceStatistics "
                           "cov[" + SizeFormatter::toString(i) + "]"
                           "[" + SizeFormatter::toString(j) + "]:\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,16) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,16));
        }
    }

    calcCov = getCovariance(std.begin(), std.end(), expCor);

    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            double calculated = calcCov[i][j],
                   expected   = expCov[i][j];
            if (QL_FABS(calculated-expected) > 1.0e-10) {
                BOOST_FAIL("getCovariance "
                           "cov[" + SizeFormatter::toString(i) + "]"
                           "[" + SizeFormatter::toString(j) + "]:\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,11) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,11));
            }
        }
    }
}


test_suite* CovarianceTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Covariance/correlation tests");
    suite->add(BOOST_TEST_CASE(&CovarianceTest::testSalvagingCorrelation));
    suite->add(BOOST_TEST_CASE(&CovarianceTest::testCovariance));
    return suite;
}


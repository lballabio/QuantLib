
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

namespace {

    Real norm(const Matrix& m) {
        Real sum = 0.0;
        for (Size i=0; i<m.rows(); i++)
            for (Size j=0; j<m.columns(); j++)
                sum += m[i][j]*m[i][j];
        return QL_SQRT(sum);
    }
}

void CovarianceTest::testSalvagingCorrelation() {

    BOOST_MESSAGE("Testing correlation-salvaging algorithms...");

    Real expected, calculated;

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
                BOOST_FAIL("SalvagingCorrelation with spectral alg "
                           "cor[" + SizeFormatter::toString(i) + "]"
                           "[" + SizeFormatter::toString(j) + "]:\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,16) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,16));
        }
    }

    Matrix badCov(n, n);
    badCov[0][0] = 0.04000; badCov[0][1] = 0.03240; badCov[0][2] = 0.02240;
    badCov[1][0] = 0.03240; badCov[1][1] = 0.03240; badCov[1][2] = 0.00864;
    badCov[2][0] = 0.02240; badCov[2][1] = 0.00864; badCov[2][2] = 0.02560;

    b = pseudoSqrt(badCov, SalvagingAlgorithm::Spectral);
    Matrix goodCov = b * transpose(b);

    Real error = norm(goodCov-badCov);
    if (error > 5.0e-4)
        BOOST_FAIL(DecimalFormatter::toExponential(error) + 
            " error while salvaging covariance matrix with spectral alg\n"
            "input matrix:\n" +
            SequenceFormatter::toString(badCov.begin(), badCov.end(), 6, 0, n)
            + "\nsalvaged matrix:\n" +
            SequenceFormatter::toString(goodCov.begin(),goodCov.end(),6,0,n));

}

void CovarianceTest::testCovariance() {

    BOOST_MESSAGE("Testing covariance calculation...");

    Real data00[] = { 3.0,  9.0 };
    Real data01[] = { 2.0,  7.0 };
    Real data02[] = { 4.0, 12.0 };
    Real data03[] = { 5.0, 15.0 };
    Real data04[] = { 6.0, 17.0 };
    Real* data[5] = { data00, data01, data02, data03, data04 };
    std::vector<Real> weights(LENGTH(data), 1.0);

    Size i, j, n = LENGTH(data00);

    Matrix expCor(n, n);
    expCor[0][0] = 1.0000000000000000; expCor[0][1] = 0.9970544855015813;
    expCor[1][0] = 0.9970544855015813; expCor[1][1] = 1.0000000000000000;

    SequenceStatistics<> s(n);
    std::vector<Real> temp(n);

    for (i = 0; i<LENGTH(data); i++) {
        for (j=0; j<n; j++) {
            temp[j]= data[i][j];
        }
        s.add(temp, weights[i]);
    }

    std::vector<Real> m = s.mean();
    std::vector<Real> std = s.standardDeviation();
    Matrix calcCov  =  s.covariance();
    Matrix calcCor  =  s.correlation();

    Matrix expCov(n, n);
    for (i=0; i<n; i++) {
        expCov[i][i] = std[i]*std[i];
        for (j=0; j<i; j++) {
            expCov[i][j] = expCov[j][i] = expCor[i][j]*std[i]*std[j];
        }
    }

    Real expected, calculated;
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
            Real calculated = calcCov[i][j],
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


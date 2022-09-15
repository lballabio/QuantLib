/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2022 Chester Wong
 Copyright (C) 2022 Wojciech Czernous

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

#include "covariance.hpp"
#include "utilities.hpp"
#include <ql/math/matrixutilities/getcovariance.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace covariance_test {

    Real norm(const Matrix& m) {
        Real sum = 0.0;
        for (Size i=0; i<m.rows(); i++)
            for (Size j=0; j<m.columns(); j++)
                sum += m[i][j]*m[i][j];
        return std::sqrt(sum);
    }

}


void CovarianceTest::testRankReduction() {

    BOOST_TEST_MESSAGE("Testing matrix rank reduction salvaging algorithms...");

    using namespace covariance_test;

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

    Matrix b = rankReducedSqrt(badCorr, 3, 1.0, SalvagingAlgorithm::Spectral);
    Matrix calcCorr = b * transpose(b);

    for (Size i=0; i<n; i++) {
        for (Size j=0; j<n; j++) {
            expected   = goodCorr[i][j];
            calculated = calcCorr[i][j];
            if (std::fabs(calculated-expected) > 1.0e-10)
                BOOST_ERROR("Salvaging correlation with spectral alg "
                            "through rankReducedSqrt "
                            << "cor[" << i << "][" << j << "]:\n"
                            << std::setprecision(10)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);
        }
    }

    Matrix badCov(n, n);
    badCov[0][0] = 0.04000; badCov[0][1] = 0.03240; badCov[0][2] = 0.02240;
    badCov[1][0] = 0.03240; badCov[1][1] = 0.03240; badCov[1][2] = 0.00864;
    badCov[2][0] = 0.02240; badCov[2][1] = 0.00864; badCov[2][2] = 0.02560;

    b = pseudoSqrt(badCov, SalvagingAlgorithm::Spectral);
    b = rankReducedSqrt(badCov, 3, 1.0, SalvagingAlgorithm::Spectral);
    Matrix goodCov = b * transpose(b);

    Real error = norm(goodCov-badCov);
    if (error > 4.0e-4)
        BOOST_ERROR(
            std::scientific << error
            << " error while salvaging covariance matrix with spectral alg "
            "through rankReducedSqrt\n"
            << std::fixed
            << "input matrix:\n" << badCov
            << "salvaged matrix:\n" << goodCov);
}

void CovarianceTest::testSalvagingMatrix() {

    BOOST_TEST_MESSAGE("Testing positive semi-definiteness salvaging "
                       "algorithms...");

    using namespace covariance_test;

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
            if (std::fabs(calculated-expected) > 1.0e-10)
                BOOST_ERROR("SalvagingCorrelation with spectral alg "
                            << "cor[" << i << "][" << j << "]:\n"
                            << std::setprecision(10)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);
        }
    }

    Matrix badCov(n, n);
    badCov[0][0] = 0.04000; badCov[0][1] = 0.03240; badCov[0][2] = 0.02240;
    badCov[1][0] = 0.03240; badCov[1][1] = 0.03240; badCov[1][2] = 0.00864;
    badCov[2][0] = 0.02240; badCov[2][1] = 0.00864; badCov[2][2] = 0.02560;

    b = pseudoSqrt(badCov, SalvagingAlgorithm::Spectral);
    Matrix goodCov = b * transpose(b);

    Real error = norm(goodCov-badCov);
    if (error > 4.0e-4)
        BOOST_ERROR(
            std::scientific << error
            << " error while salvaging covariance matrix with spectral alg\n"
            << std::fixed
            << "input matrix:\n" << badCov
            << "salvaged matrix:\n" << goodCov);
}

void CovarianceTest::testCovariance() {

    BOOST_TEST_MESSAGE("Testing covariance and correlation calculations...");

    std::vector<std::vector<Real>> data = {
        { 3.0,  9.0 },
        { 2.0,  7.0 },
        { 4.0, 12.0 },
        { 5.0, 15.0 },
        { 6.0, 17.0 }
    };
    std::vector<Real> weights(data.size(), 1.0);

    Size i, j, n = data[0].size();

    Matrix expCor(n, n);
    expCor[0][0] = 1.0000000000000000; expCor[0][1] = 0.9970544855015813;
    expCor[1][0] = 0.9970544855015813; expCor[1][1] = 1.0000000000000000;

    SequenceStatistics s(n);
    std::vector<Real> temp(n);

    for (i = 0; i<data.size(); i++) {
        for (j=0; j<n; j++) {
            temp[j]= data[i][j];
        }
        s.add(temp, weights[i]);
    }

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
            if (std::fabs(calculated-expected) > 1.0e-10)
                BOOST_ERROR("SequenceStatistics "
                            << "cor[" << i << "][" << j << "]:\n"
                            << std::setprecision(10)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);

            expected   =  expCov[i][j];
            calculated = calcCov[i][j];
            if (std::fabs(calculated-expected) > 1.0e-10)
                BOOST_ERROR("SequenceStatistics "
                            << "cov[" << i << "][" << j << "]:\n"
                            << std::setprecision(10)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);
        }
    }

    calcCov = getCovariance(std.begin(), std.end(), expCor);

    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            Real calculated = calcCov[i][j],
                 expected   = expCov[i][j];
            if (std::fabs(calculated-expected) > 1.0e-10) {
                BOOST_ERROR("getCovariance "
                            << "cov[" << i << "][" << j << "]:\n"
                            << std::setprecision(10)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);
            }
        }
    }




    CovarianceDecomposition covDecomposition(expCov);
    calcCor = covDecomposition.correlationMatrix();
    Array calcStd = covDecomposition.standardDeviations();

    for (i=0; i<n; i++) {
        calculated = calcStd[i];
        expected   = std[i];
        if (std::fabs(calculated-expected) > 1.0e-16) {
            BOOST_ERROR("CovarianceDecomposition "
                        << "standardDev[" << i << "]:\n"
                        << std::setprecision(16) << std::scientific
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected);
        }
        for (j=0; j<n; j++) {
            calculated = calcCor[i][j];
            expected   = expCor[i][j];
            if (std::fabs(calculated-expected) > 1.0e-14) {
                BOOST_ERROR("\nCovarianceDecomposition "
                            << "corr[" << i << "][" << j << "]:\n"
                            << std::setprecision(14) << std::scientific
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected);
            }
        }
    }



}

void CovarianceTest::testLRTCovariance() {

    BOOST_TEST_MESSAGE("Testing N(0,1) random generator with likelihood-ratio test for covariance ...");

    BigNatural seed = 42;
    Size dimension = 100; // length of the Gaussian sequence 
    Size samples = dimension + 3;

    PseudoRandom::rsg_type gsg = PseudoRandom::make_sequence_generator(dimension, seed);
    typedef PseudoRandom::rsg_type::sample_type sample_type;
    SequenceStatistics stats(dimension);
    for (Size j = 0; j < samples; ++j)
    {
        const sample_type& sample = gsg.nextSequence(); 
        stats.add(sample.value.begin(), sample.value.end());
    }

    Matrix expCov(dimension, dimension, 0.0); // expected covariance is the identity matrix (iid N(0,1))
    for (Size i = 0; i < dimension; ++i)
        expCov[i][i] = 1.0;

    Real calculated = stats.likelihoodratiotest(expCov);

    if (fabs(calculated) > 3)
        BOOST_ERROR("The value of the LRT statistic for covariance matrix"
                    " is: \n" << calculated << ", which is drawn from"
                    " a distribution close to N(0,1).\n"
                    "This value is outside the interval [-3,3], which is weird.");
}



test_suite* CovarianceTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Covariance and correlation tests");
    suite->add(QUANTLIB_TEST_CASE(&CovarianceTest::testCovariance));
    suite->add(QUANTLIB_TEST_CASE(&CovarianceTest::testSalvagingMatrix));
    suite->add(QUANTLIB_TEST_CASE(&CovarianceTest::testRankReduction));
    suite->add(QUANTLIB_TEST_CASE(&CovarianceTest::testLRTCovariance));
    return suite;
}


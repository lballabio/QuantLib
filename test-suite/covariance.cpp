
/*
 Copyright (C) 2003 RiskMap srl

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

#include "covariance.hpp"

using namespace QuantLib;
using QuantLib::Math::Matrix;
using QuantLib::MonteCarlo::getCovariance;

void CovarianceTest::runTest() {

    Size n = 3;

    Array vol(n);
    vol[0] = 0.1; vol[1] = 0.5; vol[2] = 1.0;

    Matrix corr(n,n);
    corr[0][0] = 1.0; corr[0][1] = 0.2; corr[0][2] = 0.5; 
    corr[1][0] = 0.2; corr[1][1] = 1.0; corr[1][2] = 0.8; 
    corr[2][0] = 0.5; corr[2][1] = 0.8; corr[2][2] = 1.0; 

    Matrix expCov(n,n);
    int i,j;
    for (i=0; i<n; i++) {
        expCov[i][i] = vol[i]*vol[i];
        for (j=0; j<i; j++) {
            expCov[i][j] = expCov[j][i] = corr[i][j]*vol[i]*vol[j];
        }
    }

    Matrix calcCov = getCovariance(vol,corr);
    
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            double calculated = calcCov[i][j],
                   expected   = expCov[i][j];
            if (QL_FABS(calculated-expected) > 1.0e-10) {
                CPPUNIT_FAIL(
                             "cov[" + IntegerFormatter::toString(i) + "]"
                             "[" + IntegerFormatter::toString(j) + "]:\n"
                             "    calculated: "
                             + DoubleFormatter::toString(calculated,11) + "\n"
                             "    expected:   " 
                             + DoubleFormatter::toString(expected,11));
            }
        }
    }
}


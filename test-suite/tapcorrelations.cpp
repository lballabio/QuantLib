/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "tapcorrelations.hpp"
#include "utilities.hpp"
#include <ql/models/marketmodels/piecewiseconstantcorrelations/tapcorrelations.hpp>
#include <ql/math/matrix.hpp>
#include <sstream>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/constraint.hpp> 

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

//QL_BEGIN_TEST_LOCALS(TapCorrelationTest)
//QL_END_TEST_LOCALS(TapCorrelationTest)

 // from Higham - nearest correlation matrix
    Matrix M5;
    void setup() {
        M5 = Matrix(4, 4);
        M5[0][0] = 2;   M5[0][1] = -1;  M5[0][2] = 0.0; M5[0][3] = 0.0;
        M5[1][0] = M5[0][1];  M5[1][1] = 2;   M5[1][2] = -1;  M5[1][3] = 0.0;
        M5[2][0] = M5[0][2]; M5[2][1] = M5[1][2];  M5[2][2] = 2;   M5[2][3] = -1;
        M5[3][0] = M5[0][3]; M5[3][1] = M5[1][3]; M5[3][2] = M5[2][3];  M5[3][3] = 2;
    }

void TapCorrelationTest::testValues() {
    
    BOOST_MESSAGE("Testing Triangular Angles Parametrization correlations matrices");
    Size rank3MatrixSize = 10;
    Real alpha = -0.419973;
    Real t0 = 136.575;
    Real epsilon = -0.00119954;
    Matrix rank3PseudoRoot 
        = triangularAnglesParametrizationRankThree(alpha, t0, epsilon, rank3MatrixSize);
    Matrix correlations = rank3PseudoRoot*transpose(rank3PseudoRoot);

    Size fullRankSize = 10;
    Array angles(fullRankSize-1);
    for (Size i = 0; i<angles.size(); ++i)
        angles[i] = M_PI/2 * Real(i+1)/Real(angles.size()+1);
    Matrix fullRankPseudoRoot 
        = triangularAnglesParametrization(angles);
    Matrix fullRankCorrelations = 
        fullRankPseudoRoot * transpose(fullRankPseudoRoot);
    BOOST_MESSAGE(correlations);
}

void TapCorrelationTest::testCalibration() {
    setup();
    LevenbergMarquardt lm;
    FrobeniusCostFunction frobeniusCostFunction(M5, 
         &triangularAnglesParametrizationUnconstrained);
    Array initialValues(M5.rows()-1, 0);
    NoConstraint constraints;
    Problem problem(frobeniusCostFunction, constraints, initialValues);
    Size maxIterations = 100;
    Size maxStationaryStateIterations = 10;
    Real rootEpsilon = 1e-4;
    Real functionEpsilon = 1e-4;
    Real gradientNormEpsilon = 1e-4;
    EndCriteria endCriteria(maxIterations, maxStationaryStateIterations, rootEpsilon, 
        functionEpsilon, gradientNormEpsilon);
    EndCriteria::Type optimizationResult;
    optimizationResult = lm.minimize(problem, endCriteria);
    Array test(3, 0);
    Matrix resultTest = triangularAnglesParametrizationUnconstrained(test);
    Matrix approximatedCorrelationsTest = resultTest * transpose(resultTest);
    Matrix result = triangularAnglesParametrizationUnconstrained(problem.currentValue());
    Matrix approximatedCorrelations = result * transpose(result);
    BOOST_MESSAGE(problem.currentValue());
}


// --- Call the desired tests
test_suite* TapCorrelationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("SMM Caplet calibration test");

    //suite->add(BOOST_TEST_CASE(&TapCorrelationTest::testValues));
    suite->add(BOOST_TEST_CASE(&TapCorrelationTest::testCalibration));

    return suite;
}

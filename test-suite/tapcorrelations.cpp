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
#include <ql/models/marketmodels/correlations/tapcorrelations.hpp>
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

    Matrix M;
    Matrix M5;

    // matrices given in Rapisarda Mercurio Brigo article
    Matrix table1;
    Matrix table2;
    Matrix table3;
    void setup() {
        Real value1 = .5;
        Real value2 = .2;
        Real value3 = .2;
        M = Matrix(3, 3);
        M[0][0] = 1.0;     M[0][1] = value1;    M[0][2] = value3;
        M[1][0] = value1;  M[1][1] = 1.0;       M[1][2] = value2;
        M[2][0] = value3;  M[2][1] = value2;    M[2][2] = 1.0;

        M5 = Matrix(4, 4);
        M5[0][0] = 2;   M5[0][1] = -1;  M5[0][2] = 0.0; M5[0][3] = 0.0;
        M5[1][0] = M5[0][1];  M5[1][1] = 2;   M5[1][2] = -1;  M5[1][3] = 0.0;
        M5[2][0] = M5[0][2]; M5[2][1] = M5[1][2];  M5[2][2] = 2;   M5[2][3] = -1;
        M5[3][0] = M5[0][3]; M5[3][1] = M5[1][3]; M5[3][2] = M5[2][3];  M5[3][3] = 2;

        table1 = Matrix(10,10);
        table1[0][0] = 1;           table1[0][1] = 0.82343;     table1[0][2] = 0.68878;     table1[0][3] = 0.585848;    table1[0][4] = 0.506972;    table1[0][5] = 0.446384;    table1[0][6] = 0.399731;    table1[0][7] = 0.363721;    table1[0][8] = 0.33586;     table1[0][9] = 0.314251;
        table1[1][0] = 0.82343;     table1[1][1] = 1;           table1[1][2] = 0.824169;    table1[1][3] = 0.68992;     table1[1][4] = 0.587171;    table1[1][5] = 0.50834;     table1[1][6] = 0.447713;    table1[1][7] = 0.400974;    table1[1][8] = 0.364855;    table1[1][9] = 0.336874;
        table1[2][0] = 0.68878;     table1[2][1] = 0.824169;    table1[2][2] = 1;           table1[2][3] = 0.82491;     table1[2][4] = 0.691064;    table1[2][5] = 0.5885;      table1[2][6] = 0.509715;    table1[2][7] = 0.449051;    table1[2][8] = 0.402227;    table1[2][9] = 0.365998;
        table1[3][0] = 0.585848;    table1[3][1] = 0.68992;     table1[3][2] = 0.82491;     table1[3][3] = 1;           table1[3][4] = 0.825651;    table1[3][5] = 0.692211;    table1[3][6] = 0.589833;    table1[3][7] = 0.511097;    table1[3][8] = 0.450397;    table1[3][9] = 0.403489;
        table1[4][0] = 0.506972;    table1[4][1] = 0.587171;    table1[4][2] = 0.691064;    table1[4][3] = 0.825651;    table1[4][4] = 1;           table1[4][5] = 0.826393;    table1[4][6] = 0.69336;     table1[4][7] = 0.591172;    table1[4][8] = 0.512487;    table1[4][9] = 0.451752;
        table1[5][0] = 0.446384;    table1[5][1] = 0.50834;     table1[5][2] = 0.5885;      table1[5][3] = 0.692211;    table1[5][4] = 0.826393;    table1[5][5] = 1;           table1[5][6] = 0.827137;    table1[5][7] = 0.694513;    table1[5][8] = 0.592516;    table1[5][9] = 0.513883;
        table1[6][0] = 0.399731;    table1[6][1] = 0.447713;    table1[6][2] = 0.509715;    table1[6][3] = 0.589833;    table1[6][4] = 0.69336;     table1[6][5] = 0.827137;    table1[6][6] = 1;           table1[6][7] = 0.827881;    table1[6][8] = 0.695668;    table1[6][9] = 0.593864;
        table1[7][0] = 0.363721;    table1[7][1] = 0.400974;    table1[7][2] = 0.449051;    table1[7][3] = 0.511097;    table1[7][4] = 0.591172;    table1[7][5] = 0.694513;    table1[7][6] = 0.827881;    table1[7][7] = 1;           table1[7][8] = 0.828626;    table1[7][9] = 0.696826;
        table1[8][0] = 0.33586;     table1[8][1] = 0.364855;    table1[8][2] = 0.402227;    table1[8][3] = 0.450397;    table1[8][4] = 0.512487;    table1[8][5] = 0.592516;    table1[8][6] = 0.695668;    table1[8][7] = 0.828626;    table1[8][8] = 1;           table1[8][9] = 0.829372;
        table1[9][0] = 0.314251;    table1[9][1] = 0.336874;    table1[9][2] = 0.365998;    table1[9][3] = 0.403489;    table1[9][4] = 0.451752;    table1[9][5] = 0.513883;    table1[9][6] = 0.593864;    table1[9][7] = 0.696826;    table1[9][8] = 0.829372;    table1[9][9] = 1;

        table2 = Matrix(10,10);
        table2[0][0] = 1;           table2[0][1] = 0.945456;    table2[0][2] = 0.785792;    table2[0][3] = 0.649755;    table2[0][4] = 0.54892;     table2[0][5] = 0.474984;    table2[0][6] = 0.420199;    table2[0][7] = 0.379106;    table2[0][8] = 0.347965;    table2[0][9] = 0.324174;
        table2[1][0] = 0.945456;    table2[1][1] = 1;           table2[1][2] = 0.852651;    table2[1][3] = 0.660103;    table2[1][4] = 0.5197;      table2[1][5] = 0.423721;    table2[1][6] = 0.357031;    table2[1][7] = 0.30945;     table2[1][8] = 0.274717;    table2[1][9] = 0.248907;
        table2[2][0] = 0.785792;    table2[2][1] = 0.852651;    table2[2][2] = 1;           table2[2][3] = 0.945427;    table2[2][4] = 0.865675;    table2[2][5] = 0.801713;    table2[2][6] = 0.754902;    table2[2][7] = 0.720887;    table2[2][8] = 0.69589;     table2[2][9] = 0.677266;
        table2[3][0] = 0.649755;    table2[3][1] = 0.660103;    table2[3][2] = 0.945427;    table2[3][3] = 1;           table2[3][4] = 0.981399;    table2[3][5] = 0.952491;    table2[3][6] = 0.927228;    table2[3][7] = 0.907276;    table2[3][8] = 0.891878;    table2[3][9] = 0.880029;
        table2[4][0] = 0.54892;     table2[4][1] = 0.5197;      table2[4][2] = 0.865675;    table2[4][3] = 0.981399;    table2[4][4] = 1;           table2[4][5] = 0.993236;    table2[4][6] = 0.981815;    table2[4][7] = 0.970965;    table2[4][8] = 0.961801;    table2[4][9] = 0.954358;
        table2[5][0] = 0.474984;    table2[5][1] = 0.423721;    table2[5][2] = 0.801713;    table2[5][3] = 0.952491;    table2[5][4] = 0.993236;    table2[5][5] = 1;           table2[5][6] = 0.997202;    table2[5][7] = 0.992108;    table2[5][8] = 0.986934;    table2[5][9] = 0.98234;
        table2[6][0] = 0.420199;    table2[6][1] = 0.357031;    table2[6][2] = 0.754902;    table2[6][3] = 0.927228;    table2[6][4] = 0.981815;    table2[6][5] = 0.997202;    table2[6][6] = 1;           table2[6][7] = 0.998698;    table2[6][8] = 0.996186;    table2[6][9] = 0.993513;
        table2[7][0] = 0.379106;    table2[7][1] = 0.30945;     table2[7][2] = 0.720887;    table2[7][3] = 0.907276;    table2[7][4] = 0.970965;    table2[7][5] = 0.992108;    table2[7][6] = 0.998698;    table2[7][7] = 1;           table2[7][8] = 0.999338;    table2[7][9] = 0.99801;
        table2[8][0] = 0.347965;    table2[8][1] = 0.274717;    table2[8][2] = 0.69589;     table2[8][3] = 0.891878;    table2[8][4] = 0.961801;    table2[8][5] = 0.986934;    table2[8][6] = 0.996186;    table2[8][7] = 0.999338;    table2[8][8] = 1;           table2[8][9] = 0.999642;
        table2[9][0] = 0.324174;    table2[9][1] = 0.248907;    table2[9][2] = 0.677266;    table2[9][3] = 0.880029;    table2[9][4] = 0.954358;    table2[9][5] = 0.98234;     table2[9][6] = 0.993513;    table2[9][7] = 0.99801;     table2[9][8] = 0.999642;    table2[9][9] = 1;

        table3 = Matrix(10,10);
        table3[0][0] = 1;           table3[0][1] = 0.978243;    table3[0][2] = 0.914671;    table3[0][3] = 0.813954;    table3[0][4] = 0.683046;    table3[0][5] = 0.530338;    table3[0][6] = 0.364789;    table3[0][7] = 0.195157;    table3[0][8] = 0.029425;    table3[0][9] = -0.125574;
        table3[1][0] = 0.978243;    table3[1][1] = 1;           table3[1][2] = 0.978625;    table3[1][3] = 0.916746;    table3[1][4] = 0.819617;    table3[1][5] = 0.694389;    table3[1][6] = 0.54928;     table3[1][7] = 0.392787;    table3[1][8] = 0.233051;    table3[1][9] = 0.077401;
        table3[2][0] = 0.914671;    table3[2][1] = 0.978625;    table3[2][2] = 1;           table3[2][3] = 0.979296;    table3[2][4] = 0.919861;    table3[2][5] = 0.827313;    table3[2][6] = 0.708783;    table3[2][7] = 0.572146;    table3[2][8] = 0.425348;    table3[2][9] = 0.275891;
        table3[3][0] = 0.813954;    table3[3][1] = 0.916746;    table3[3][2] = 0.979296;    table3[3][3] = 1;           table3[3][4] = 0.980205;    table3[3][5] = 0.92379;     table3[3][6] = 0.836525;    table3[3][7] = 0.725343;    table3[3][8] = 0.597665;    table3[3][9] = 0.460843;
        table3[4][0] = 0.683046;    table3[4][1] = 0.819617;    table3[4][2] = 0.919861;    table3[4][3] = 0.980205;    table3[4][4] = 1;           table3[4][5] = 0.981288;    table3[4][6] = 0.92828;     table3[4][7] = 0.846713;    table3[4][8] = 0.743194;    table3[4][9] = 0.624625;
        table3[5][0] = 0.530338;    table3[5][1] = 0.694389;    table3[5][2] = 0.827313;    table3[5][3] = 0.92379;     table3[5][4] = 0.981288;    table3[5][5] = 1;           table3[5][6] = 0.98248;     table3[5][7] = 0.933086;    table3[5][8] = 0.857378;    table3[5][9] = 0.761553;
        table3[6][0] = 0.364789;    table3[6][1] = 0.54928;     table3[6][2] = 0.708783;    table3[6][3] = 0.836525;    table3[6][4] = 0.92828;     table3[6][5] = 0.98248;     table3[6][6] = 1;           table3[6][7] = 0.983722;    table3[6][8] = 0.937995;    table3[6][9] = 0.868101;
        table3[7][0] = 0.195157;    table3[7][1] = 0.392787;    table3[7][2] = 0.572146;    table3[7][3] = 0.725343;    table3[7][4] = 0.846713;    table3[7][5] = 0.933086;    table3[7][6] = 0.983722;    table3[7][7] = 1;           table3[7][8] = 0.984965;    table3[7][9] = 0.942838;
        table3[8][0] = 0.029425;    table3[8][1] = 0.233051;    table3[8][2] = 0.425348;    table3[8][3] = 0.597665;    table3[8][4] = 0.743194;    table3[8][5] = 0.857378;    table3[8][6] = 0.937995;    table3[8][7] = 0.984965;    table3[8][8] = 1;           table3[8][9] = 0.986173;
        table3[9][0] = -0.125574;   table3[9][1] = 0.077401;    table3[9][2] = 0.275891;    table3[9][3] = 0.460843;    table3[9][4] = 0.624625;    table3[9][5] = 0.761553;    table3[9][6] = 0.868101;    table3[9][7] = 0.942838;    table3[9][8] = 0.986173;    table3[9][9] = 1;

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
        = lmmTriangularAnglesParametrization(angles);
    Matrix fullRankCorrelations =
        fullRankPseudoRoot * transpose(fullRankPseudoRoot);
    BOOST_MESSAGE(correlations);
}

void TapCorrelationTest::testCalibration() {
    setup();
    LevenbergMarquardt lm;
    FrobeniusCostFunction frobeniusCostFunction(M,
         &lmmTriangularAnglesParametrizationUnconstrained);
    Array initialValues(M.rows()-1, 0);
    NoConstraint constraints;
    Problem problem(frobeniusCostFunction, constraints, initialValues);
    Size maxIterations = 1000;
    Size maxStationaryStateIterations = 100;
    Real rootEpsilon = 1e-8;
    Real functionEpsilon = 1e-8;
    Real gradientNormEpsilon = 1e-8;
    EndCriteria endCriteria(maxIterations, maxStationaryStateIterations, rootEpsilon,
        functionEpsilon, gradientNormEpsilon);
    EndCriteria::Type optimizationResult;
    optimizationResult = lm.minimize(problem, endCriteria);
    Array currentValue = problem.currentValue();
    Matrix approximatedPseudoRoot
        = lmmTriangularAnglesParametrizationUnconstrained(currentValue);

    Matrix approximatedCorrelations
        = approximatedPseudoRoot * transpose(approximatedPseudoRoot);
    /*Array test(2, 0);
    Matrix resultTest = lmmTriangularAnglesParametrizationUnconstrained(test);
    Matrix approximatedCorrelationsTest = resultTest * transpose(resultTest);
    Matrix result = lmmTriangularAnglesParametrizationUnconstrained(problem.currentValue());
    Matrix approximatedCorrelations = result * transpose(result);*/
    BOOST_MESSAGE(problem.currentValue());
}


// --- Call the desired tests
test_suite* TapCorrelationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("SMM Caplet calibration test");

    //suite->add(BOOST_TEST_CASE(&TapCorrelationTest::testValues));
    suite->add(BOOST_TEST_CASE(&TapCorrelationTest::testCalibration));

    return suite;
}

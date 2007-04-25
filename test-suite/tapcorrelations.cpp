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

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

//QL_BEGIN_TEST_LOCALS(TapCorrelationTest)
//QL_END_TEST_LOCALS(TapCorrelationTest)



void TapCorrelationTest::testValues() {
    
    BOOST_MESSAGE("Testing Triangular Angles Parametrization correlations matrices");
    Size rank3MatrixSize = 10;
    Matrix rank3PseudoRoot(rank3MatrixSize,3);
    Real alpha = -0.419973;
    Real t0 = 136.575;
    Real epsilon = -0.00119954;
    setTriangularAnglesParametrizationRankThree(alpha, t0, epsilon, rank3PseudoRoot);
    Matrix correlations = rank3PseudoRoot*transpose(rank3PseudoRoot);

    Size fullRankSize = 10;
    Matrix fullRankPseudoRoot(fullRankSize, fullRankSize);
    std::vector<Real> angles(fullRankSize-1);
    for (Size i = 0; i<angles.size(); ++i)
        angles[i] = M_PI/2 * Real(i+1)/Real(angles.size()+1);
    setTriangularAnglesParametrization(angles, fullRankPseudoRoot);
    Matrix fullRankCorrelations = 
        fullRankPseudoRoot * transpose(fullRankPseudoRoot);
    BOOST_MESSAGE(correlations);
}


// --- Call the desired tests
test_suite* TapCorrelationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("SMM Caplet calibration test");

    suite->add(BOOST_TEST_CASE(&TapCorrelationTest::testValues));

    return suite;
}

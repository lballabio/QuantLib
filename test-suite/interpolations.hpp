/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2009 Dimitri Reiswich

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

#ifndef quantlib_test_interpolations_hpp
#define quantlib_test_interpolations_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class InterpolationTest {
  public:
    // cubic spline tests
    static void testSplineOnGenericValues();
    static void testSimmetricEndConditions();
    static void testDerivativeEndConditions();
    static void testNonRestrictiveHymanFilter();
    static void testSplineOnRPN15AValues();
    static void testSplineOnGaussianValues();
    static void testSplineErrorOnGaussianValues();
    static void testMultiSpline();
    static void testAsFunctor();
    // other interpolations
    static void testBackwardFlat();
    static void testForwardFlat();
    static void testSabrInterpolation();
    static void testKernelInterpolation();
    static void testKernelInterpolation2D();
    static void testBicubicDerivatives();
    static void testBicubicUpdate();
    static void testRichardsonExtrapolation();

    static boost::unit_test_framework::test_suite* suite();
};


#endif

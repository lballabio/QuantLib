/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2008, 2009 StatPro Italia srl

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

#ifndef quantlib_test_piecewise_yield_curve_hpp
#define quantlib_test_piecewise_yield_curve_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class PiecewiseYieldCurveTest {
  public:
    static void testLogCubicDiscountConsistency();
    static void testLogLinearDiscountConsistency();
    static void testLinearDiscountConsistency();

    static void testLogLinearZeroConsistency();
    static void testLinearZeroConsistency();
    static void testSplineZeroConsistency();

    static void testLinearForwardConsistency();
    static void testFlatForwardConsistency();
    static void testSplineForwardConsistency();

    static void testConvexMonotoneForwardConsistency();
    static void testLocalBootstrapConsistency();

    static void testObservability();
    static void testLiborFixing();

    static void testJpyLibor();

    static void testDiscountCopy();
    static void testForwardCopy();
    static void testZeroCopy();

    static boost::unit_test_framework::test_suite* suite();
};


#endif

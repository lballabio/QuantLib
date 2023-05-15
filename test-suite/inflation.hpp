/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon

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

#ifndef quantlib_test_inflation_hpp
#define quantlib_test_inflation_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
 comment block of the corresponding class */

class InflationTest {
public:
    static void testPeriod();
    static void testZeroIndex();
    static void testZeroTermStructure();
    static void testSeasonalityCorrection();
    static void testZeroIndexFutureFixing();
    static void testInterpolatedZeroTermStructure();
    static void testQuotedYYIndex();
    static void testRatioYYIndex();
    static void testOldRatioYYIndex();
    static void testYYTermStructure();
    static void testCpiFlatInterpolation();
    static void testCpiLinearInterpolation();
    static void testCpiAsIndexInterpolation();
    static boost::unit_test_framework::test_suite* suite();
};


#endif


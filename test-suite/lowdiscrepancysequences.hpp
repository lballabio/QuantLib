
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#ifndef quantlib_test_lowdiscrepancysequences_hpp
#define quantlib_test_lowdiscrepancysequences_hpp

#include <boost/test/unit_test.hpp>

class LowDiscrepancyTest {
  public:
    static void testPolynomialsModuloTwo();
    static void testSobol();
    static void testHalton();
    static void testTrueRandomNumberDiscrepancy();
    static void testMersenneTwisterDiscrepancy();
    static void testPlainHaltonDiscrepancy();
    static void testRandomStartHaltonDiscrepancy();
    static void testRandomShiftHaltonDiscrepancy();
    static void testRandomStartRandomShiftHaltonDiscrepancy();
    static void testJackelSobolDiscrepancy();
    static void testUnitSobolDiscrepancy();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

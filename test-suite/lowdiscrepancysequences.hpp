
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

/*! \class QuantLib::SobolRsg <ql/RandomNumbers/sobolrsg.hpp>

    \test a) the correctness of the returned values is tested by
          reproducing known good values.

    \test b) the correctness of the returned values is tested by
          checking their discrepancy against known good values.
*/

/*! \class QuantLib::HaltonRsg <ql/RandomNumbers/haltonrsg.hpp>

    \test a) the correctness of the returned values is tested by
          reproducing known good values.

    \test b) the correctness of the returned values is tested by
          checking their discrepancy against known good values.
*/

/*! \class QuantLib::FaureRsg <ql/RandomNumbers/faurersg.hpp>

    \test a) the correctness of the returned values is tested by
          reproducing known good values.

*/

class LowDiscrepancyTest {
  public:
    static void testPolynomialsModuloTwo();
    static void testSobol();
    static void testHalton();
    static void testFaure();
    static void testMersenneTwisterDiscrepancy();
    static void testPlainHaltonDiscrepancy();
    static void testRandomStartHaltonDiscrepancy();
    static void testRandomShiftHaltonDiscrepancy();
    static void testRandomStartRandomShiftHaltonDiscrepancy();
    static void testJackelSobolDiscrepancy();
    static void testSobolLevitanSobolDiscrepancy();
    static void testUnitSobolDiscrepancy();

    static boost::unit_test_framework::test_suite* suite();
};


#endif

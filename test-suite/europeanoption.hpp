/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

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

#ifndef quantlib_test_european_option_hpp
#define quantlib_test_european_option_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class EuropeanOptionTest {
  public:
    static void testValues();
    static void testGreekValues();
    static void testGreeks();
    static void testImpliedVol();
    static void testImpliedVolContainment();
    static void testJRBinomialEngines();
    static void testCRRBinomialEngines();
    static void testEQPBinomialEngines();
    static void testTGEOBinomialEngines();
    static void testTIANBinomialEngines();
    static void testLRBinomialEngines();
    static void testJOSHIBinomialEngines();
    static void testFdEngines();
    static void testIntegralEngines();
    static void testQmcEngines();
    static void testMcEngines();
    static void testFFTEngines();
    static void testLocalVolatility();
    static void testAnalyticEngineDiscountCurve();
    static void testPDESchemes();
    static void testDouglasVsCrankNicolson();
    static void testFdEngineWithNonConstantParameters();

    static boost::unit_test_framework::test_suite* suite();
    static boost::unit_test_framework::test_suite* experimental();
};


#endif

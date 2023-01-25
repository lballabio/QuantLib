/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2005 StatPro Italia srl

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

#ifndef quantlib_test_american_option_hpp
#define quantlib_test_american_option_hpp

#include <boost/test/unit_test.hpp>
#include "speedlevel.hpp"

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class AmericanOptionTest {
  public:
    static void testBaroneAdesiWhaleyValues();
    static void testBjerksundStenslandValues();
    static void testJuValues();
    static void testFdValues();
    static void testFdAmericanGreeks();
    static void testFdShoutGreeks();
    static void testFDShoutNPV();
    static void testZeroVolFDShoutNPV();
    static void testLargeDividendShoutNPV();
    static void testEscrowedVsSpotAmericanOption();
    static void testTodayIsDividendDate();
    static void testCallPutParity();
    static void testQdPlusBoundaryValues();
    static void testQdPlusBoundaryConvergence();
    static void testQdAmericanEngines();
    static void testQdFpIterationScheme();
    static void testAndersenLakeHighPrecisionExample();
    static void testQdEngineStandardExample();
    static void testBulkQdFpAmericanEngine();
    static void testQdEngineWithLobattoIntegral();
    static void testQdNegativeDividendYield();
    static void testBjerksundStenslandEuorpeanGreeks();
    static void testBjerksundStenslandAmericanGreeks();
    static void testSingleBjerksundStenslandGreeks();

    static boost::unit_test_framework::test_suite* suite(SpeedLevel);
};


#endif

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#ifndef quantlib_test_asian_options_hpp
#define quantlib_test_asian_options_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class AsianOptionTest {
  public:
    static void testAnalyticContinuousGeometricAveragePrice();
    static void testAnalyticContinuousGeometricAveragePriceGreeks();
    static void testAnalyticDiscreteGeometricAveragePrice();
    static void testAnalyticDiscreteGeometricAverageStrike();
    static void testMCDiscreteGeometricAveragePrice();
    static void testMCDiscreteArithmeticAveragePrice();
    static void testMCDiscreteArithmeticAverageStrike();
    static void testAnalyticDiscreteGeometricAveragePriceGreeks();
    static void testPastFixings();
    static void testLevyEngine();
    static boost::unit_test_framework::test_suite* suite();
    static boost::unit_test_framework::test_suite* experimental();
};


#endif

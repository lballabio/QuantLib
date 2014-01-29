/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

#ifndef quantlib_test_catbond_hpp
#define quantlib_test_catbond_hpp

#include <boost/test/unit_test.hpp>

class CatBondTest {
  public:
    static void testEventSetForWholeYears();
    static void testEventSetForIrregularPeriods();
    static void testEventSetForNoEvents();
    static void testBetaRisk();
    static void testRiskFreeAgainstFloatingRateBond();
    static void testCatBondInDoomScenario();
    static void testCatBondWithDoomOnceInTenYears();
    static void testCatBondWithDoomOnceInTenYearsProportional();
    static void testCatBondWithGeneratedEventsProportional();
    static boost::unit_test_framework::test_suite* suite();
};

#endif

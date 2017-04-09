/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2012 Peter Caspers

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

#ifndef quantlib_test_market_model_hpp
#define quantlib_test_market_model_hpp

#include <ql/qldefines.hpp>
#include <boost/test/unit_test.hpp>
#include "speedlevel.hpp"

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class MarketModelTest {
  public:
    enum MarketModelType { ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility/*, CalibratedMM*/
    };
    static void testInverseFloater();
    static void testPeriodAdapter();
    static void testAllMultiStepProducts();
    static void testOneStepForwardsAndOptionlets();
    static void testOneStepNormalForwardsAndOptionlets();
    static void testCallableSwapNaif();
    static void testCallableSwapLS();
    static void testCallableSwapAnderson(
        MarketModelType marketModel, unsigned testedFactor);
    static void testGreeks();
    static void testPathwiseGreeks();
    static void testPathwiseVegas();
    static void testPathwiseMarketVegas();
    static void testStochVolForwardsAndOptionlets();
    static void testAbcdVolatilityIntegration();
    static void testAbcdVolatilityCompare();
    static void testAbcdVolatilityFit();
    static void testDriftCalculator();
    static void testIsInSubset();
    static void testAbcdDegenerateCases();
    static void testCovariance();
    static boost::unit_test_framework::test_suite* suite(SpeedLevel);
};

#endif

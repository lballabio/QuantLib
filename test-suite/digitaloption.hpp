
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth

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

#ifndef quantlib_test_digital_option_hpp
#define quantlib_test_digital_option_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::AnalyticEuropeanEngine \
    <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>

    \test a) the correctness of the returned value in case of
          cash-or-nothing digital payoff is tested by reproducing
          results available in literature.

    \test b) the correctness of the returned value in case of
          asset-or-nothing digital payoff is tested by reproducing
          results available in literature.

    \test c) the correctness of the returned value in case of gap
          digital payoff is tested by reproducing results available in
          literature.

    \test d) the correctness of the returned greeks in case of
          cash-or-nothing digital payoff is tested by reproducing
          numerical derivatives.
*/

/*! \class QuantLib::AnalyticDigitalAmericanEngine \
    <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>

    \test a) the correctness of the returned value in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.

    \test b) the correctness of the returned value in case of
          asset-or-nothing at-hit digital payoff is tested by
          reproducing results available in literature.

    \test c) the correctness of the returned value in case of
          cash-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.

    \test d) the correctness of the returned value in case of
          asset-or-nothing at-expiry digital payoff is tested by
          reproducing results available in literature.

    \test e) the correctness of the returned greeks in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing numerical derivatives.
*/

/*! \class QuantLib::MCDigitalEngine \
    <ql/PricingEngines/Vanilla/mcdigitalengine.hpp>

    \test the correctness of the returned value in case of
          cash-or-nothing at-hit digital payoff is tested by
          reproducing known good results.
*/

class DigitalOptionTest {
  public:
    static void testCashOrNothingEuropeanValues();
    static void testAssetOrNothingEuropeanValues();
    static void testGapEuropeanValues();
    static void testCashAtHitOrNothingAmericanValues();
    static void testAssetAtHitOrNothingAmericanValues();
    static void testCashAtExpiryOrNothingAmericanValues();
    static void testAssetAtExpiryOrNothingAmericanValues();
    static void testCashAtHitOrNothingAmericanGreeks();
    static void testMCCashAtHit();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

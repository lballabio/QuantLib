
/*
 Copyright (C) 2003 RiskMap srl

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

#ifndef quantlib_test_european_option_hpp
#define quantlib_test_european_option_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::AnalyticEuropeanEngine \
    <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>

    \test a) the correctness of the returned value is tested by
          reproducing results available in literature.

    \test b) the correctness of the returned greeks is tested by
          reproducing results available in literature.

    \test c) the correctness of the returned greeks is tested by
          reproducing numerical derivatives.

    \test d) the correctness of the returned implied volatility is tested
          by using it for reproducing the target value.

    \test e) the implied-volatility calculation is tested
          by checking that it does not modify the option.
*/

/*! \class QuantLib::BinomialVanillaEngine \
    <ql/PricingEngines/Vanilla/binomialengine.hpp>

    \test the correctness of the returned value is tested by checking
          it against analytic results.
*/

/*! \class QuantLib::MCEuropeanEngine \
    <ql/PricingEngines/Vanilla/mceuropeanengine.hpp>

    \test the correctness of the returned value is tested by checking
          it against analytic results.
*/

class EuropeanOptionTest {
  public:
    static void testValues();
    static void testGreekValues();
    static void testGreeks();
    static void testImpliedVol();
    static void testImpliedVolContainment();
    static void testBinomialEngines();
    static void testMcEngines();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

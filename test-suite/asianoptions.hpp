
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

#ifndef quantlib_test_asian_options_hpp
#define quantlib_test_asian_options_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::AnalyticContinuousAveragingAsianEngine \
    <ql/PricingEngines/Asian/analyticcontinuousasianengine.hpp>

    \test a) the correctness of the returned value is tested by
          reproducing results available in literature.

    \test b) the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
*/

/*! \class QuantLib::AnalyticDiscreteAveragingAsianEngine \
    <ql/PricingEngines/Asian/analyticdiscreteasianengine.hpp>

    \test a) the correctness of the returned value is tested by
          reproducing results available in literature.

    \test b) the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
*/

class AsianOptionTest {
  public:
    static void testGeometricContinuousAverage();
    static void testGeometricContinuousGreeks();
    static void testGeometricDiscreteAverage();
    static void testGeometricDiscreteGreeks();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

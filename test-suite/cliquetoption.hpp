
/*
 Copyright (C) 2004 StatPro Italia srl

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

#ifndef quantlib_test_cliquet_option_hpp
#define quantlib_test_cliquet_option_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::AnalyticCliquetEngine \
    <ql/PricingEngines/Cliquet/analyticcliquetengine.hpp>

    \test a) the correctness of the returned value is tested by
          reproducing results available in literature.

    \test b) the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
*/

/*! \class QuantLib::AnalyticPerformanceEngine \
    <ql/PricingEngines/Cliquet/analyticperformanceengine.hpp>

    \test the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
*/

class CliquetOptionTest {
  public:
    static void testValues();
    static void testGreeks();
    static void testPerformanceGreeks();
    static boost::unit_test_framework::test_suite* suite();
};


#endif


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

#ifndef quantlib_test_exchange_rate_hpp
#define quantlib_test_exchange_rate_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::ExchangeRate <ql/exchangerate.hpp>

    \test application of direct and derived exchange rate is tested
          against calculations.
*/

/*! \class QuantLib::ExchangeRateMamager \
    <ql/Currencies/exchangeratemanager.hpp>

    \test lookup of direct, triangulated, and derived exchange rates
          is tested.
*/

class ExchangeRateTest {
  public:
    static void testDirect();
    static void testDerived();
    static void testDirectLookup();
    static void testTriangulatedLookup();
    static void testSmartLookup();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

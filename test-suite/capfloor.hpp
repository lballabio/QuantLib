
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

#ifndef quantlib_test_cap_floor_hpp
#define quantlib_test_cap_floor_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::CapFloor <ql/Instruments/capfloor.hpp>

    \test a) the correctness of the returned value is tested by checking
          that the price of a cap (resp. floor) decreases
          (resp. increases) with the strike rate.

    \test b) the relationship between the values of caps, floors and
          the resulting collars is checked.

    \test c) the put-call parity between the values of caps, floors and
          swaps is checked.

    \test d) the correctness of the returned implied volatility is tested
          by using it for reproducing the target value.

    \test e) the correctness of the returned value is tested by checking
          it against a known good value.
*/

class CapFloorTest {
  public:
    static void testStrikeDependency();
    static void testConsistency();
    static void testParity();
    static void testImpliedVolatility();
    static void testCachedValue();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

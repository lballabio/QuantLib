
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

#ifndef quantlib_test_swap_hpp
#define quantlib_test_swap_hpp

#include <boost/test/unit_test.hpp>

/*! \class QuantLib::SimpleSwap <ql/Instruments/simpleswap.hpp>

    \test a) the correctness of the returned value is tested by
          checking that the price of a swap paying the fair fixed rate
          is null.

    \test b) the correctness of the returned value is tested by
          checking that the price of a swap receiving the fair
          floating-rate spread is null.

    \test c) the correctness of the returned value is tested by
          checking that the price of a swap decreases with the
          paid fixed rate.

    \test d) the correctness of the returned value is tested by
          checking that the price of a swap increases with the
          received floating-rate spread.

    \test e) the correctness of the returned value is tested by checking
          it against a known good value.
*/

class SwapTest {
  public:
    static void testFairRate();
    static void testFairSpread();
    static void testRateDependency();
    static void testSpreadDependency();
    static void testCachedValue();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

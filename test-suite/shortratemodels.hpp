/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#ifndef quantlib_test_short_rate_models_hpp
#define quantlib_test_short_rate_models_hpp

#include <boost/test/unit_test.hpp>
#include "speedlevel.hpp"

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class ShortRateModelTest {
  public:
    static void testFuturesConvexityBias();
    static void testCachedHullWhite();
    static void testCachedHullWhiteFixedReversion();
    static void testCachedHullWhite2();
    static void testSwaps();
    static boost::unit_test_framework::test_suite* suite(SpeedLevel);
};


#endif

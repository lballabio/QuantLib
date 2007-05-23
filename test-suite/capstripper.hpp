/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_capsstripper_hpp
#define quantlib_test_capsstripper_hpp

#include <boost/test/unit_test.hpp>

/*! To do: test against known good values
*/

class CapsStripperTest {
  public:
    static void FlatVolatilityStripping();
    static void highPrecisionTest();
    static void testSpreadedStripper();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

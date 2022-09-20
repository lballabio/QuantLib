/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl
 Copyright (C) 2021, 2022 Ralf Konrad Eckel

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

#ifndef quantlib_test_callable_bonds_hpp
#define quantlib_test_callable_bonds_hpp

#include <boost/test/unit_test.hpp>

class CallableBondTest {
  public:
    static void testConsistency();
    static void testInterplay();
    static void testObservability();
    static void testDegenerate();
    static void testCached();
    static void testSnappingExerciseDate2ClosestCouponDate();
    static void testImpliedVol();
    static boost::unit_test_framework::test_suite* suite();
};


#endif

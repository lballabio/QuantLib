/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_preconditions_hpp
#define quantlib_test_preconditions_hpp

#include <boost/test/unit_test.hpp>
#include <ql/qldefines.hpp>

// Deprecated in version 1.43.
enum [[deprecated("Not maintained going forward; run the whole test suite or use --run_test to select a subset of tests")]] SpeedLevel {
    Slow   = 0,
    Fast   = 1,
    Faster = 2
};

QL_DEPRECATED_DISABLE_WARNING

// Deprecated in version 1.43.
struct [[deprecated("Not maintained going forward; run the whole test suite or use --run_test to select a subset of tests")]] if_speed {
    SpeedLevel speed;

    if_speed(SpeedLevel speed);

    boost::test_tools::assertion_result operator()(boost::unit_test::test_unit_id);
};

QL_DEPRECATED_ENABLE_WARNING

struct usingAtParCoupons {
    boost::test_tools::assertion_result operator()(boost::unit_test::test_unit_id);
};

#endif //quantlib_test_preconditions_hpp

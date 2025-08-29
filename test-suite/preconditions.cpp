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

#include "preconditions.hpp"
#include "quantlibglobalfixture.hpp"
#include <ql/cashflows/iborcoupon.hpp>

using namespace QuantLib;
namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

if_speed::if_speed(SpeedLevel speed) : speed(speed) {}

tt::assertion_result if_speed::operator()(utf::test_unit_id) {
    tt::assertion_result level (QuantLibGlobalFixture::get_speed() <= speed);
    level.message() << "precondition failed";
    return level;
}

tt::assertion_result usingAtParCoupons::operator()(boost::unit_test::test_unit_id) {
    return tt::assertion_result(IborCoupon::Settings::instance().usingAtParCoupons());
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 StatPro Italia srl

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

#include "speedlevel.hpp"
#include "quantlibglobalfixture.hpp"

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

if_speed::if_speed(SpeedLevel speed) : speed(speed) {}

tt::assertion_result if_speed::operator()(utf::test_unit_id) {
    tt::assertion_result level (QuantLibGlobalFixture::get_speed() <= speed);
    level.message() << "precondition failed";
    return level;
}

SpeedLevel speed_level(int argc, char** argv) {
    /*! Again, dead simple parser:
        - passing --slow causes all tests to be run;
        - passing --fast causes most tests to be run, except the slowest;
        - passing --faster causes only the faster tests to be run;
        - passing nothing is the same as --slow
    */

    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--slow")
            return Slow;
        else if (arg == "--fast")
            return Fast;
        else if (arg == "--faster")
            return Faster;
    }
    return Slow;
}
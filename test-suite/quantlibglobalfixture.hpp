/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl

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

#ifndef quantlib_global_fixture_hpp
#define quantlib_global_fixture_hpp

#include "preconditions.hpp"
#include <chrono>

class QuantLibGlobalFixture {  // NOLINT(cppcoreguidelines-special-member-functions)
  public:
    QuantLibGlobalFixture();
    ~QuantLibGlobalFixture();
    QL_DEPRECATED_DISABLE_WARNING
    // Deprecated in version 1.43.
    [[deprecated("Not maintained going forward; run the whole test suite or use --run_test to select a subset of tests")]]
    static SpeedLevel get_speed ();
    QL_DEPRECATED_ENABLE_WARNING

  private:
    decltype(std::chrono::steady_clock::now()) start;
    decltype(std::chrono::steady_clock::now()) stop;
};

QL_DEPRECATED_DISABLE_WARNING
// Deprecated in version 1.43.
[[deprecated("Not maintained going forward; run the whole test suite or use --run_test to select a subset of tests")]]
SpeedLevel speed_level (int argc, char **argv);
QL_DEPRECATED_ENABLE_WARNING

#endif

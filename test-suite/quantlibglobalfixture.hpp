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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_global_fixture_hpp
#define quantlib_global_fixture_hpp

#include "speedlevel.hpp"
#include <chrono>

class QuantLibGlobalFixture {  // NOLINT(cppcoreguidelines-special-member-functions)
  public:
    QuantLibGlobalFixture();
    ~QuantLibGlobalFixture();
    static SpeedLevel get_speed ();
    SpeedLevel speed_level (int argc, char **argv);

  private:
    static SpeedLevel speed;
    decltype(std::chrono::steady_clock::now()) start;
    decltype(std::chrono::steady_clock::now()) stop;
};

#endif

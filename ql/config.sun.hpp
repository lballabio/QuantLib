/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl

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

#ifndef quantlib_config_sun_hpp
#define quantlib_config_sun_hpp

#include <ql/userconfig.hpp>

/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

// to be compatible with gcc and VC++, make math functions
// available in the global namespace as well.
#include <cmath>
using std::sqrt; using std::abs; using std::fabs;
using std::exp; using std::log; using std::pow;
using std::sin; using std::cos; using std::asin; using std::acos;
using std::sinh; using std::cosh;
using std::floor; using std::fmod; using std::modf;


#endif

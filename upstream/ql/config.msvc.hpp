/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2010 StatPro Italia srl
 Copyright (C) 2004, 2008 Ferdinando Ametrano

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

#ifndef quantlib_config_msvc_hpp
#define quantlib_config_msvc_hpp

#include <ql/userconfig.hpp>

// first things first
#if (_MSC_VER < 1910)
#  error "versions of Visual C++ prior to VC++ 2017 are no longer supported"
#endif

/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

// force undefining min and max macros
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

// conditionally work around compiler glitches
#define QL_PATCH_MSVC

// Compilation on the x64 platform throws a lot of warnings assigning
// QuantLib::Size == size_t (64 bit) to QuantLib::Integer == int (32
// bit under x64 Windows). We disable this warning until a better
// solution is suggested.
#ifdef _M_X64
#pragma warning(disable : 4267)
#endif


/* suppress C++ code analysis warning C26812 in VS 2019:
 * Prefer 'enum class' over 'enum' (Enum.3). */
#pragma warning(disable : 26812)


#ifndef _CPPRTTI
#   error Enable Run-Time Type Info (Property Pages | C/C++ | Language)
#endif

#endif

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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_config_msvc_hpp
#define quantlib_config_msvc_hpp

#include <ql/userconfig.hpp>

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

// leave outside here common configs

#define QL_PATCH_MSVC  // more granularity below

// prevent auto-link of Boost libs such as serialization
#define BOOST_ALL_NO_LIB

// select toolset:
#if (_MSC_VER < 1400)
#  error "versions of Visual C++ prior to VC++8 (2005) are not supported"
#elif (_MSC_VER == 1400)
// move inside here configs specific to VC++ 8 (2005)
#  ifndef _SCL_SECURE_NO_DEPRECATE
#    define _SCL_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  define QL_PATCH_MSVC80
#  define QL_WORKING_BOOST_STREAMS
// see the corresponding pragmas in the 7.1 section
#  pragma warning(disable: 4267)
#  pragma warning(disable: 4224)
// non-ASCII characters - Disabling this warning here is ineffective
// and the change has been made instead under project properties
//#  pragma warning(disable: 4819)
#elif (_MSC_VER == 1500)
// move inside here configs specific to VC++ 9 (2008)
#  ifndef _SCL_SECURE_NO_DEPRECATE
#    define _SCL_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  define QL_PATCH_MSVC90
#  define QL_WORKING_BOOST_STREAMS
// see the corresponding pragmas in the 7.1 section
#  pragma warning(disable: 4267)
#  pragma warning(disable: 4224)
#elif (_MSC_VER == 1600)
// move inside here configs specific to VC++ 10 (2010)
#  define QL_PATCH_MSVC100
#  define QL_WORKING_BOOST_STREAMS
#else
// Microsoft compiler we still don't know about.
// We treat it the same way as the latest and hope for the better.
#  define QL_PATCH_MSVC100
#  define QL_WORKING_BOOST_STREAMS
#endif

// Compilation on the x64 platform throws a lot of warnings assigning
// QuantLib::Size == size_t (64 bit) to QuantLib::Integer == int (32
// bit under x64 Windows). We disable this warning until a better
// solution is suggested.
#ifdef _M_X64
#pragma warning(disable : 4267)
#endif

#ifndef _CPPRTTI
#   if (_MSC_VER >= 1300) // VC++ 7.0 (.Net) and later
#       error Enable Run-Time Type Info (Property Pages | C/C++ | Language)
#   else
#       error Enable Run-Time Type Information (Project Settings | C/C++ | C++ Language)
#   endif
#endif

#endif

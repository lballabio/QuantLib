
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_config_msvc_h
#define quantlib_config_msvc_h

#include <ql/userconfig.hpp>

/* For Microsoft Visual C++, uncomment the following lines to unmask
   floating-point exceptions. Warning: unpredictable results can arise...

   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
   Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }


/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

// disable useless warning
// 'identifier' : decorated name length exceeded,
//                name was truncated in debug info
#pragma warning(disable: 4786)
#pragma warning(disable: 4503)


// leave outside here common configs

#define HAVE_CSTDLIB 1
#define HAVE_CMATH   1

/* Useful constants missing in Visual C++ math.h  */
#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.434294481903251827651
#define M_IVLN10    0.434294481903251827651 /* 1 / log(10) */
#define M_LN2       0.693147180559945309417
#define M_LOG2_E    0.693147180559945309417
#define M_LN10      2.30258509299404568402
#define M_PI        3.141592653589793238462643383280
#define M_TWOPI     (M_PI * 2.0)
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_3PI_4     2.3561944901923448370E0
#define M_SQRTPI    1.77245385090551602792981
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076
#define M_1_SQRTPI  0.564189583547756286948
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT_2    0.7071067811865475244008443621048490392848359376887
#define M_SQRT1_2   0.7071067811865475244008443621048490392848359376887
#define M_LN2LO     1.9082149292705877000E-10
#define M_LN2HI     6.9314718036912381649E-1
#define M_SQRT3     1.73205080756887719000
#define M_INVLN2    1.4426950408889633870E0  /* 1 / log(2) */


#define HAVE_CTIME   1
#define HAVE_CCTYPE  1

#define QL_PATCH_MSVC  // more granularity below

//
// select toolset:
//
#if (_MSC_VER < 1200)
#  error "unsupported Microsoft compiler"
#elif (_MSC_VER == 1200)
// move inside here configs specific to VC++ 6.0
#  define QL_PATCH_MSVC6
#  define CHOKES_ON_TYPENAME
#  define HAVE_INCOMPLETE_ITERATOR_SUPPORT
#  define REQUIRES_DUMMY_RETURN
#  define QL_LIB_TOOLSET "vc6"
#elif (_MSC_VER == 1300)
// move inside here configs specific to VC++ 7.0 (.Net)
// not sure the following two are really needed
#  define HAVE_INCOMPLETE_ITERATOR_SUPPORT
#  define REQUIRES_DUMMY_RETURN
#  define QL_LIB_TOOLSET "vc7"
#elif (_MSC_VER == 1310)
// move inside here configs specific to VC++ 7.1 (.Net 2003)
#  define QL_LIB_TOOLSET "vc71"
#else
#  define QL_LIB_TOOLSET "vc" BOOST_STRINGIZE(_MSC_VER)
#endif


/*** libraries to be linked ***/

//
// select thread opt:
//
#ifdef _MT
#  define QL_LIB_THREAD_OPT "-mt"
#else
#  define QL_LIB_THREAD_OPT
#endif
//
// select linkage opt:
//
#ifdef _DLL
#  if defined(_DEBUG)
#    define QL_LIB_RT_OPT "-gd"
#  else
#    define QL_LIB_RT_OPT
#  endif
#else
#  if defined(_DEBUG)
#    define QL_LIB_RT_OPT "-sgd"
#  else
#    define QL_LIB_RT_OPT "-s"
#  endif
#endif

#define QL_LIB_NAME "QuantLib-" QL_LIB_TOOLSET QL_LIB_THREAD_OPT QL_LIB_RT_OPT "-" QL_LIB_VERSION ".lib"

#pragma comment(lib, QL_LIB_NAME)
#ifdef BOOST_LIB_DIAGNOSTIC
#  pragma message("Linking to lib file: " QL_LIB_NAME)
#endif

#ifndef _CPPRTTI
#   if (_MSC_VER >= 1300) // VC++ 7.0 (.Net) and later
#       error Enable Run-Time Type Info (Property Pages | C/C++ | Language)
#   else
#       error Enable Run-Time Type Information (Project Settings | C/C++ | C++ Language)
#   endif
#endif

#endif


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

#ifndef quantlib_config_bcc_h
#define quantlib_config_bcc_h

#include <ql/userconfig.hpp>

/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

#define HAVE_CSTDLIB 1
#define HAVE_CMATH   1

/* Useful constants missing in Borland math.h  */
#define M_IVLN10    0.434294481903251827651 /* 1 / log(10) */
#define M_LOG2_E    0.693147180559945309417
#define M_TWOPI     (M_PI * 2.0)
#define M_3PI_4     2.3561944901923448370E0
#define M_SQRTPI    1.77245385090551602792981
#define M_SQRT1_2   0.7071067811865475244008443621048490392848359376887
#define M_LN2LO     1.9082149292705877000E-10
#define M_LN2HI     6.9314718036912381649E-1
#define M_SQRT3     1.73205080756887719000
#define M_INVLN2    1.4426950408889633870E0  /* 1 / log(2) */

#define HAVE_CTIME   1
#define HAVE_CCTYPE  1

#define QL_MIN       std::min
#define QL_MAX       std::max

#define QL_PATCH_BORLAND

#ifndef QL_DEBUG
//#  pragma defineonoption QL_DEBUG -v
#endif

#  define QL_LIB_TOOLSET "bcb"
//
// select thread opt:
//
#ifdef __MT__
#  define QL_LIB_THREAD_OPT "-mt"
#else
#  define QL_LIB_THREAD_OPT
#endif
//
// select linkage opt:
//
#  ifdef _RTLDLL
#     ifdef _DEBUG
#         define QL_LIB_RT_OPT "-d"
#     else
#         define QL_LIB_RT_OPT
#     endif
#  else
#     ifdef _DEBUG
#         define QL_LIB_RT_OPT "-sd"
#     else
#         define QL_LIB_RT_OPT "-s"
#     endif
#  endif

#define QL_LIB_NAME "QuantLib-" QL_LIB_TOOLSET QL_LIB_THREAD_OPT QL_LIB_RT_OPT "-" QL_LIB_VERSION ".lib"

#pragma comment(lib, QL_LIB_NAME)
#ifdef BOOST_LIB_DIAGNOSTIC
#  pragma message(QL_LIB_NAME)
#endif

// should check here for -RT
//#ifndef _CPPRTTI
//    #error Check the 'C/C++ | C++ Language | Use RTTI' option
//#endif


#endif

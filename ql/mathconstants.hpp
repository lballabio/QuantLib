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

#ifndef quantlib_math_constants_hpp
#define quantlib_math_constants_hpp

#include <cmath>

#ifndef M_E
    #define M_E         2.71828182845904523536
#endif

#ifndef M_LOG2E
    #define M_LOG2E     1.44269504088896340736
#endif

#ifndef M_LOG10E
    #define M_LOG10E    0.434294481903251827651
#endif

#ifndef M_IVLN10
    #define M_IVLN10    0.434294481903251827651
#endif

#ifndef M_LN2
    #define M_LN2       0.693147180559945309417
#endif

#ifndef M_LOG2_E
    #define M_LOG2_E    0.693147180559945309417
#endif

#ifndef M_LN10
    #define M_LN10      2.30258509299404568402
#endif

#ifndef M_PI
#    define M_PI 3.141592653589793238462643383280
#endif

#ifndef M_TWOPI
    #define M_TWOPI     (M_PI * 2.0)
#endif

#ifndef M_PI_2
    #define M_PI_2      1.57079632679489661923
#endif

#ifndef M_PI_4
    #define M_PI_4      0.785398163397448309616
#endif

#ifndef M_3PI_4
    #define M_3PI_4     2.3561944901923448370E0
#endif

#ifndef M_SQRTPI
    #define M_SQRTPI    1.77245385090551602792981
#endif

#ifndef M_1_PI
    #define M_1_PI      0.318309886183790671538
#endif

#ifndef M_2_PI
    #define M_2_PI      0.636619772367581343076
#endif

#ifndef M_1_SQRTPI
    #define M_1_SQRTPI  0.564189583547756286948
#endif

#ifndef M_2_SQRTPI
    #define M_2_SQRTPI  1.12837916709551257390
#endif

#ifndef M_SQRT2
    #define M_SQRT2     1.41421356237309504880
#endif

#ifndef M_SQRT_2
    #define M_SQRT_2    0.7071067811865475244008443621048490392848359376887
#endif

#ifndef M_SQRT1_2
    #define M_SQRT1_2   0.7071067811865475244008443621048490392848359376887
#endif

#ifndef M_LN2LO
    #define M_LN2LO     1.9082149292705877000E-10
#endif

#ifndef M_LN2HI
    #define M_LN2HI     6.9314718036912381649E-1
#endif

#ifndef M_SQRT3
    #define M_SQRT3     1.73205080756887719000
#endif

#ifndef M_INVLN2
    #define M_INVLN2    1.4426950408889633870E0
#endif

/* This should ensure that no macro are redefined if we happen to
   include <math.h> again, whether or not we're using our macros
   or theirs. We can't know in advance, since it depends on the
   order of inclusion of headers in client code. */
#ifdef _MSC_VER
    #undef _USE_MATH_DEFINES
#endif

#endif



#ifndef id_2a5717b5087ce77d85ef6ac2f834609c
#define id_2a5717b5087ce77d85ef6ac2f834609c
inline bool test_2a5717b5087ce77d85ef6ac2f834609c(int* i) { return i != 0; }
#endif

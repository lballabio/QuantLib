
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

#ifndef quantlib_config_ansi_h
#define quantlib_config_ansi_h

#include <ql/userconfig.hpp>

/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

#define HAVE_CSTDLIB 1
#define QL_SIZE_T    std::size_t
#define QL_ATOI      std::atoi

#define HAVE_CMATH   1
#define QL_SQRT      std::sqrt
#define QL_FABS      std::fabs
#define QL_EXP       std::exp
#define QL_LOG       std::log
#define QL_SIN       std::sin
#define QL_COS       std::cos
#define QL_SINH      std::sinh
#define QL_COSH      std::cosh
#define QL_POW       std::pow
#define QL_MODF      std::modf
#define QL_FLOOR     std::floor

#define M_SQRT_2    0.7071067811865475244008443621048490392848359376887
#define M_1_SQRTPI  0.564189583547756286948
#define M_SQRTPI    1.77245385090551602792981

#define HAVE_LIMITS  1

#define HAVE_CTIME   1
#define QL_TIME_T    std::time_t
#define QL_TM        std::tm
#define QL_CLOCK     std::clock
#define QL_TIME      std::time
#define QL_GMTIME    std::gmtime

#define HAVE_CCTYPE  1
#define QL_TOLOWER   std::tolower
#define QL_TOUPPER   std::toupper

#define HAVE_CSTRING 1
#define QL_STRLEN    std::strlen

#define QL_MIN       std::min
#define QL_MAX       std::max

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif

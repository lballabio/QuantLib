
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// $Id$

#ifndef quantlib_config_ansi_h
#define quantlib_config_ansi_h

#define HAVE_CSTDLIB
#define QL_SIZE_T std::size_t

#define HAVE_CMATH
#define QL_SQRT   std::sqrt
#define QL_FABS   std::fabs
#define QL_EXP    std::exp
#define QL_LOG    std::log
#define QL_SIN    std::sin
#define QL_COS    std::cos
#define QL_POW    std::pow
#define QL_MODF   std::modf
#define QL_FLOOR  std::floor

#define HAVE_LIMITS

#define HAVE_CTIME
#define QL_CLOCK    std::clock
#define QL_TIME     std::time

#define HAVE_CCTYPE
#define QL_STRLEN   std::strlen
#define QL_TOLOWER  std::tolower
#define QL_TOUPPER  std::toupper

#define HAVE_CSTDIO
#define QL_SPRINTF  std::sprintf

#define QL_MIN      std::min
#define QL_MAX      std::max

#define HAVE_EXPRESSION_TEMPLATES
#define HAVE_TEMPLATE_METAPROGRAMMING

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif

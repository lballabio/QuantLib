/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/

// $Id$

#ifndef quantlib_config_decc_h
#define quantlib_config_decc_h

#define HAVE_STDLIB_H
#define QL_SIZE_T size_t

#define HAVE_MATH_H
#define QL_SQRT   sqrt
#define QL_FABS   fabs
#define QL_EXP    exp
#define QL_LOG    log
#define QL_SIN    sin
#define QL_COS    cos
#define QL_POW    pow
#define QL_MODF   modf

#define HAVE_LIMITS

#define HAVE_TIME_H
#define QL_CLOCK  clock
#define QL_TIME   time

#define HAVE_CTYPE_H
#define QL_STRLEN   strlen
#define QL_TOLOWER  tolower
#define QL_TOUPPER  toupper

#define HAVE_STDIO_H
#define QL_SPRINTF  sprintf

#define QL_ITERATOR         std::iterator
#define QL_ITERATOR_TRAITS  std::iterator_traits


#endif

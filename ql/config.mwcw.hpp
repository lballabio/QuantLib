
/*
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

#ifndef quantlib_config_mwcw_h
#define quantlib_config_mwcw_h

#include <ql/userconfig.hpp>

/*******************************************
   System configuration section:
   do not modify the following definitions.
 *******************************************/

#define REQUIRES_DUMMY_RETURN

#define HAVE_CSTDLIB 1
#define HAVE_CMATH   1
#define HAVE_CTIME   1
#define HAVE_CCTYPE  1

#define QL_MIN       std::min
#define QL_MAX       std::max


#endif

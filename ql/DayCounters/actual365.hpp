
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

/*! \file actual365.hpp
    \brief act/365 day counter
*/

#ifndef quantlib_actual365_day_counter_h
#define quantlib_actual365_day_counter_h

#include <ql/daycounter.hpp>

namespace QuantLib {

  #ifndef QL_DISABLE_DEPRECATED
    /*! Actual/365 day count convention

        As per ISDA documentation Actual/365 is the same as Actual/Actual.
        You should use the ActualActual class instead, or maybe you want to
        use the Actual365Fixed class.

        \deprecated use ActualActual or Actual365Fixed instead

        \ingroup daycounters
    */
    typedef Actual365 Actual365Fixed;
  #endif

#endif

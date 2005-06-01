/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Quantlib

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file callschedule.hpp
    \brief Schedule of call dates
*/


#ifndef quantlib_call_schedule_hpp
#define quantlib_call_schedule_hpp

#include <ql/date.hpp>
#include <vector>

namespace QuantLib {

  class Price{
  public:
    enum Type { Dirty, Clean};
    Real price;
    Type type;
};
 
  class Callability {
  public:
    enum Type { Call, Put};
    Price price;
    Type type;
    Date date;
  };
 
    typedef std::vector< Callability > CallabilitySchedule;
}


#endif

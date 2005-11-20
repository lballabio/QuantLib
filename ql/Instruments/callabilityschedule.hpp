/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2005 Theo Boafo

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

/*! \file callabilityschedule.hpp
    \brief Schedule of put/call dates
*/

#ifndef quantlib_callability_schedule_hpp
#define quantlib_callability_schedule_hpp

#include <ql/date.hpp>
#include <ql/event.hpp>
#include <vector>

namespace QuantLib {

    class Price {
      public:
        enum Type { Dirty, Clean };
        Real price_;
        Type type_;
    };

    class Callability : public Event {
      public:
        enum Type { Call, Put };
        Price price_;
        Type type_;
        Date date_;
        Date date() {
            return date_;
        }
    };

    typedef std::vector<Callability> CallabilitySchedule;

}


#endif

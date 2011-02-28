/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file weekendsonly.hpp
    \brief Weekends-only calendar
*/

#ifndef quantlib_target_weekends_only_hpp
#define quantlib_target_weekends_only_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Weekends-only calendar
    /*! This calendar has no bank holidays except for weekends
        (Saturdays and Sundays) as required by ISDA for calculating
        conventional CDS spreads.

        \ingroup calendars
    */
    class WeekendsOnly : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "weekends only"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        WeekendsOnly();
    };

}


#endif

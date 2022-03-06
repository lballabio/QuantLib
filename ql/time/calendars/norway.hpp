/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file norway.hpp
    \brief Norwegian calendar
*/

#ifndef quantlib_norwegian_calendar_hpp
#define quantlib_norwegian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Norwegian calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Holy Thursday</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension</li>
        <li>Whit(Pentecost) Monday </li>
        <li>New Year's Day, January 1st</li>
        <li>May Day, May 1st</li>
        <li>National Independence Day, May 17th</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Norway : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Norway"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        Norway();
    };

}


#endif


#ifndef id_86dafbfb8fa92d58bfd5f2ca8343952c
#define id_86dafbfb8fa92d58bfd5f2ca8343952c
inline bool test_86dafbfb8fa92d58bfd5f2ca8343952c(int* i) { return i != 0; }
#endif

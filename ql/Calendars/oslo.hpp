
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

/*! \file oslo.hpp
    \brief Oslo calendar
*/

#ifndef quantlib_oslo_calendar_h
#define quantlib_oslo_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %Oslo calendar
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
      <li>National Independence Day, May 17st</li>
      <li>Christmas, December 25th</li>
      <li>Boxing Day, December 26th</li>
      </ul>
    */
    class Oslo : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Oslo"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        Oslo()
        : Calendar(boost::shared_ptr<Calendar::Impl>(new Oslo::Impl)) {}
    };

}


#endif


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

/*! \file target.hpp
    \brief TARGET calendar
*/

#ifndef quantlib_target_calendar_h
#define quantlib_target_calendar_h

#include <ql/calendar.hpp>

namespace QuantLib {

    //! %TARGET calendar
    /*! Holidays:
      <ul>
      <li>Saturdays</li>
      <li>Sundays</li>
      <li>New Year's Day, January 1st</li>
      <li>Good Friday (since 2000)</li>
      <li>Easter Monday (since 2000)</li>
      <li>Labour Day, May 1st (since 2000)</li>
      <li>Christmas, December 25th</li>
      <li>Day of Goodwill, December 26th (since 2000)</li>
      <li>December 31st (1998, 1999, and 2001)</li>
      </ul>
    */
    class TARGET : public Calendar {
      private:
        class Impl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "TARGET"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        TARGET()
        : Calendar(boost::shared_ptr<Calendar::Impl>(new TARGET::Impl)) {}
    };

}


#endif

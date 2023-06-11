/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file bespokecalendar.hpp
    \brief Bespoke calendar
*/

#ifndef quantlib_bespoke_calendar_hpp
#define quantlib_bespoke_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Bespoke calendar
    /*! This calendar has no predefined set of business days. Holidays
        and weekdays can be defined by means of the provided
        interface. Instances constructed by copying remain linked to
        the original one; adding a new holiday or weekday will affect
        all linked instances.

        \ingroup calendars
    */
    class BespokeCalendar : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            explicit Impl(std::string name = "");
            std::string name() const override;
            bool isWeekend(Weekday) const override;
            bool isBusinessDay(const Date&) const override;
            void addWeekend(Weekday);
          private:
            std::string name_;
            unsigned int weekend_mask_ = 0;
        };
        ext::shared_ptr<BespokeCalendar::Impl> bespokeImpl_;
      public:
        /*! \warning different bespoke calendars created with the same
                     name (or different bespoke calendars created with
                     no name) will compare as equal.
        */
        BespokeCalendar(const std::string& name = "");
        //! marks the passed day as part of the weekend
        void addWeekend(Weekday);
    };

}


#endif

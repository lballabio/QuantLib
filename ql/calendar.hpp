/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2004 StatPro Italia srl

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

/*! \file calendar.hpp
    \brief %calendar class
*/

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include <ql/date.hpp>
#include <ql/Patterns/bridge.hpp>
#include <set>

namespace QuantLib {

    //! Business Day conventions
    /*! These conventions specify the algorithm used to adjust a date in case
        it is not a valid business day.

        \ingroup datetime
    */
    enum BusinessDayConvention {
        Unadjusted,         /*!< Do not adjust. */
        Preceding,          /*!< Choose the first business day before
                                 the given holiday. */
        ModifiedPreceding,  /*!< Choose the first business day before
                                 the given holiday unless it belongs to a
                                 different month, in which case choose the
                                 first business day after the holiday. */
        Following,          /*!< Choose the first business day after
                                 the given holiday. */
        ModifiedFollowing,  /*!< Choose the first business day after
                                 the given holiday unless it belongs to a
                                 different month, in which case choose the
                                 first business day before the holiday. */
        MonthEndReference   /*!< Choose the first business day after
                                 the given holiday, if the original date falls
                                 on last business day of month result reverts
                                 to first business day before month-end */
    };

    //! abstract base class for calendar implementations
    class CalendarImpl {
      public:
        virtual ~CalendarImpl() {}
        virtual std::string name() const = 0;
        virtual bool isBusinessDay(const Date&) const = 0;
        std::set<Date> addedHolidays, removedHolidays;
    };

    //! %calendar class
    /*! This class provides methods for determining whether a date is a
        business day or a holiday for a given market, and for
        incrementing/decrementing a date of a given number of business days.

        The Bridge pattern is used to provide the base behavior of the
        calendar, namely, to determine whether a date is a business day.

		A calendar should be defined for specific exchange holiday schedule
		or for general country holiday schedule. Legacy city holiday schedule
		calendars will be moved to the exchange/country convention.

        \ingroup datetime

        \test the methods for adding and removing holidays are tested
              by inspecting the calendar before and after their
              invocation.
    */
    class Calendar : public Bridge<Calendar,CalendarImpl> {
      public:
        //! \name Calendar interface
        //@{
        //! Returns the name of the calendar.
        /*! \warning This method is used for output and comparison between
                calendars. It is <b>not</b> meant to be used for writing
                switch-on-type code.
        */
        std::string name() const;
        /*! Returns <tt>true</tt> iff the date is a business day for the
            given market.
        */
        bool isBusinessDay(const Date& d) const;
        /*! Returns <tt>true</tt> iff the date is a holiday for the given
            market.
        */
        bool isHoliday(const Date& d) const;
        /*! Returns <tt>true</tt> iff the date is last business day for the
            month in given market.
        */
        bool isEndOfMonth(const Date& d) const;

        /*! Adds a date to the set of holidays for the given calendar. */
        void addHoliday(const Date&);
        /*! Removes a date from the set of holidays for the given calendar. */
        void removeHoliday(const Date&);

        /*! Adjusts a non-business day to the appropriate near business day
            with respect to the given convention.
        */
        Date adjust(const Date&,
                    BusinessDayConvention convention = Following,
                    const Date& origin = Date()) const;
        /*! Advances the given date of the given number of business days and
            returns the result.
            \note The input date is not modified.
        */
        Date advance(const Date&,
                     Integer n,
                     TimeUnit unit,
                     BusinessDayConvention convention = Following) const;
        /*! Advances the given date as specified by the given period and
            returns the result.
            \note The input date is not modified.
        */
        Date advance(const Date& date,
                     const Period& period,
                     BusinessDayConvention convention = Following) const;
        //@}

        //! partial calendar implementation
        /*! This class provides the means of determining the Easter
            Monday for a given year.
        */
        class WesternImpl : public CalendarImpl {
          protected:
            //! expressed relative to first day of year
            static Day easterMonday(Year y);
        };
        /*! This default constructor returns a calendar with a null
            implementation, which is therefore unusable except as a
            placeholder.
        */
        Calendar() {}
      protected:
        /*! This protected constructor will only be invoked by derived
            classes which define a given Calendar implementation */
        Calendar(const boost::shared_ptr<CalendarImpl>& impl)
        : Bridge<Calendar,CalendarImpl>(impl) {}
    };

    /*! Returns <tt>true</tt> iff the two calendars belong to the same
        derived class.
        \relates Calendar
    */
    bool operator==(const Calendar&, const Calendar&);

    /*! \relates Calendar */
    bool operator!=(const Calendar&, const Calendar&);


    // inline definitions

    inline std::string Calendar::name() const {
        return impl_->name();
    }

    inline bool Calendar::isBusinessDay(const Date& d) const {
        if (impl_->addedHolidays.find(d) != impl_->addedHolidays.end())
            return false;
        if (impl_->removedHolidays.find(d) != impl_->removedHolidays.end())
            return true;
        return impl_->isBusinessDay(d);
    }

    inline bool Calendar::isEndOfMonth(const Date& d) const {
        return (d.month() != adjust(d+1).month());
    }

    inline bool Calendar::isHoliday(const Date& d) const {
        return !isBusinessDay(d);
    }

    inline bool operator==(const Calendar& c1, const Calendar& c2) {
        return (c1.isNull() && c2.isNull())
            || (!c1.isNull() && !c2.isNull() && c1.name() == c2.name());
    }

    inline bool operator!=(const Calendar& c1, const Calendar& c2) {
        return !(c1 == c2);
    }

}


#endif

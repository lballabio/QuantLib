
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file calendar.hpp
    \brief %calendar class

    \fullpath
    ql/%calendar.hpp
*/

// $Id$

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include <ql/date.hpp>
#include <ql/handle.hpp>
#include <ql/null.hpp>

/*! \namespace QuantLib::Calendars
    \brief Specialized Calendar classes

    See sect. \ref calendars
*/

namespace QuantLib {

    //! Rolling conventions
    /*! These conventions specify the algorithm used to find the business day
        which is "closest" to a given holiday.
    */
    enum RollingConvention {
        Preceding,          /*!< Choose the first business day before
                                 the given holiday. */
        ModifiedPreceding,  /*!< Choose the first business day before
                                 the given holiday unless it belongs to a
                                 different month, in which case choose the
                                 first business day after the holiday. */
        Following,          /*!< Choose the first business day after
                                 the given holiday. */
        ModifiedFollowing   /*!< Choose the first business day after
                                 the given holiday unless it belongs to a
                                 different month, in which case choose the
                                 first business day before the holiday. */
    };

    //! %calendar class
    /*! This class provides methods for determining whether a date is a
        business day or a holiday for a given market, and for
        incrementing/decrementing a date of a given number of business days.

        The Strategy pattern is used to provide the base behavior of the
        calendar, namely, to determine whether a date is a business day.
    */
    class Calendar {
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
        /*! Returns the next business day on the given market with respect to
            the given date and convention.
        */
        Date roll(const Date&,
                  RollingConvention convention = Following) const;
        /*! Advances the given date of the given number of business days and
            returns the result.
            \note The input date is not modified.
        */
        Date advance(const Date&, int n, TimeUnit unit,
                     RollingConvention convention = Following) const;
        //@}

        //! abstract base class for calendar factories
        class CalendarFactory {
          public:
            virtual Calendar create() const = 0;
        };
        typedef CalendarFactory factory;

        //! abstract base class for calendar implementations
        class CalendarImpl {
          public:
            virtual std::string name() const = 0;
            virtual bool isBusinessDay(const Date&) const = 0;
        };
        /*! partial implementation providing the means of
            determining the Easter Monday for a given year. */
        class WesternCalendarImpl : public CalendarImpl {
          protected:
            //! expressed relative to first day of year
            static Day easterMonday(Year y);
        };
      protected:
        /*! this protected constructor will only be invoked by derived
            classes which define a given Calendar implementation */
        Calendar(const Handle<CalendarImpl>& impl) : impl_(impl) {}
      private:
        Handle<CalendarImpl> impl_;
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
        return impl_->isBusinessDay(d);
    }

    inline bool Calendar::isHoliday(const Date& d) const {
        return !(impl_->isBusinessDay(d));
    }

    inline bool operator==(const Calendar& h1, const Calendar& h2) {
            return (h1.name() == h2.name());
    }

    inline bool operator!=(const Calendar& h1, const Calendar& h2) {
            return (h1.name() != h2.name());
    }

}


#endif

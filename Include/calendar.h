
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file calendar.h
    \brief Abstract calendar class

    $Source$
    $Name$
    $Log$
    Revision 1.7  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.6  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

/*! \namespace QuantLib::Calendars
    \brief Concrete implementations of the Calendar interface
*/

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include "qldefines.h"
#include "qlerrors.h"
#include "date.h"
#include "handle.h"
#include <string>

namespace QuantLib {

    //! Abstract calendar class
    /*! This class is purely abstract and defines the interface of concrete
        calendar classes which will be derived from this one.

        It provides methods for determining whether a date is a business day or a
        holiday for a given market, and for incrementing/decrementing a date of a
        given number of business days.
    */
    class Calendar {
      public:
        //! Returns the name of the calendar.
        /*!    \warning This method is used for output and comparison between calendars.
            It is <b>not</b> meant to be used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        //! Returns <tt>true</tt> iff the date is a business day for the given market.
        virtual bool isBusinessDay(const Date&) const = 0;
        //! Returns <tt>true</tt> iff the date is a holiday for the given market.
        bool isHoliday(const Date& d) const { return !isBusinessDay(d); }
        //! Returns the next business day on the given market with respect to the given date and convention.
        Date roll(const Date&, bool modified = false) const;
        //! Advances the given date of the givn number of business days and returns the result.
        /*! \note The input date is not modified.
        */
        Date advance(const Date&, int n, TimeUnit unit, bool modified = false) const;
    };

    QL_DECLARE_TEMPLATE_SPECIALIZATION(
    bool operator==(const Handle<Calendar>&, const Handle<Calendar>&))
    QL_DECLARE_TEMPLATE_SPECIALIZATION(
    bool operator!=(const Handle<Calendar>&, const Handle<Calendar>&))

    namespace Calendars {

        //! A calendar with no holidays, not even saturdays and sundays.
        class NullCalendar : public Calendar {
          public:
            NullCalendar() {}
            //! returns "None"
            std::string name() const { return "None"; }
            //! always returns <tt>true</tt>
            bool isBusinessDay(const Date& d) const { d; return true; }
        };

    }

    // inline definitions

    /*! Returns <tt>true</tt> iff the two calendars belong to the same derived class.
        \relates Calendar
    */
    QL_TEMPLATE_SPECIALIZATION
    inline bool operator==(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
        return (h1->name() == h2->name());
    }

    /*! \relates Calendar */
    QL_TEMPLATE_SPECIALIZATION
    inline bool operator!=(const Handle<Calendar>& h1, const Handle<Calendar>& h2) {
        return (h1->name() != h2->name());
    }

}


#endif

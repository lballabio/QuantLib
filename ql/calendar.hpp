
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

    \fullpath
    Include/ql/%calendar.hpp
    \brief Abstract calendar class

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:54:20  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.11  2001/08/31 15:23:44  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.10  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.9  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.8  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.5  2001/07/05 12:35:09  enri
// - added some static_cast<int>() to prevent gcc warnings
// - added some virtual constructor (same reason)
//
// Revision 1.4  2001/05/29 15:12:47  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_calendar_h
#define quantlib_calendar_h

#include "ql/date.hpp"
#include "ql/handle.hpp"
#include "ql/null.hpp"

/*! \namespace QuantLib::Calendars
    \brief Concrete implementations of the Calendar interface
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

    //! Abstract calendar class
    /*! This class is purely abstract and defines the interface of concrete
        calendar classes which will be derived from this one.

        It implements methods for determining whether a date is a business day
        or a holiday for a given market, and for incrementing/decrementing a
        date of a given number of business days. 
    */
    class Calendar {
      public:
        //! Returns the name of the calendar.
        /*! \warning This method is used for output and comparison between
            calendars.
            It is <b>not</b> meant to be used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        /*! Returns <tt>true</tt> iff the date is a business day for the given
            market.
        */
        virtual bool isBusinessDay(const Date&) const = 0;
        /*! Returns <tt>true</tt> iff the date is a holiday for the given
            market.
        */
        bool isHoliday(const Date& d) const { return !isBusinessDay(d); }
        /*! Returns the next business day on the given market with respect to
            the given date and convention.
        */
        Date roll(const Date&, RollingConvention convention = Following) const;
        /*! Advances the given date of the given number of business days and
            returns the result.
            \note The input date is not modified.
        */
        Date advance(const Date&, int n, TimeUnit unit,
            RollingConvention convention = Following) const;
    };

    bool operator==(const Handle<Calendar>&, const Handle<Calendar>&);
    bool operator!=(const Handle<Calendar>&, const Handle<Calendar>&);

    namespace Calendars {

        //! A calendar with no holidays, not even saturdays and sundays.
        class NullCalendar : public Calendar {
          public:
            NullCalendar() {}
            //! returns "None"
            std::string name() const { return "None"; }
            //! always returns <tt>true</tt>
            bool isBusinessDay(const Date& d) const {return true; }
        };

    }

    // inline definitions

    /*! Returns <tt>true</tt> iff the two calendars belong to the same derived
        class.
        \relates Calendar
    */
    inline bool operator==(const Handle<Calendar>& h1,
        const Handle<Calendar>& h2) {
            return (h1->name() == h2->name());
    }

    /*! \relates Calendar */
    inline bool operator!=(const Handle<Calendar>& h1,
        const Handle<Calendar>& h2) {
            return (h1->name() != h2->name());
    }

}


#endif

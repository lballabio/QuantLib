
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

/*! \file daycounter.hpp
    \fullpath Include/ql/%daycounter.hpp
    \brief Abstract day counter class

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.7  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.6  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.4  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_day_counter_h
#define quantlib_day_counter_h

#include "ql/date.hpp"
#include "ql/handle.hpp"
#include "ql/null.hpp"

/*! \namespace QuantLib::DayCounters
    \brief Concrete implementations of the DayCounter interface
*/

namespace QuantLib {

    //! Abstract day counter class
    /*! This class is purely abstract and defines the interface of concrete
        day counter classes which will be derived from this one.

        It provides methods for determining the length of a time period
        according to a number of market conventions, both as a number of days
        and as a year fraction.
    */
    class DayCounter {
      public:
        //! Returns the name of the day counter.
        /*! \warning This method is used for output and comparison between day
            counters.
            It is <b>not</b> meant to be used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        //! Returns the number of days between two dates.
        virtual int dayCount(const Date&, const Date&) const = 0;
        //! Returns the period between two dates as a fraction of year.
        virtual Time yearFraction(const Date&, const Date&,
          const Date& refPeriodStart = Date(),
          const Date& refPeriodEnd = Date()) const = 0;
    };

    // comparison based on name
    bool operator==(const Handle<DayCounter>&, 
        const Handle<DayCounter>&);
    bool operator!=(const Handle<DayCounter>&, 
        const Handle<DayCounter>&);

    // inline definitions

    /*! Returns <tt>true</tt> iff the two day counters belong to the same
        derived class.
        \relates DayCounter
    */
    inline bool operator==(const Handle<DayCounter>& h1,
        const Handle<DayCounter>& h2) {
            return (h1->name() == h2->name());
    }

    /*! \relates DayCounter */
    inline bool operator!=(const Handle<DayCounter>& h1,
        const Handle<DayCounter>& h2) {
            return (h1->name() != h2->name());
    }

}


#endif


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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file daycounter.h
    \brief Abstract day counter class

    $Source$
    $Log$
    Revision 1.9  2001/03/12 17:35:09  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.8  2001/02/16 15:19:52  lballabio
    Used QL_DECLARE_TEMPLATE_SPECIFICATIONS macro

    Revision 1.7  2001/02/09 19:21:09  lballabio
    removed QL_DECLARE_TEMPLATE_SPECIALIZATION macro

    Revision 1.6  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.5  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

/*! \namespace QuantLib::DayCounters
    \brief Concrete implementations of the DayCounter interface
*/

#ifndef quantlib_day_counter_h
#define quantlib_day_counter_h

#include "qldefines.h"
#include "date.h"
#include "handle.h"
#include "null.h"

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
    bool operator==(const Handle<DayCounter>&, const Handle<DayCounter>&);
    bool operator!=(const Handle<DayCounter>&, const Handle<DayCounter>&);

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

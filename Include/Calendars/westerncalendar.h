
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

/*! \file westerncalendar.h
    \brief Western calendar

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.4  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_western_calendar_h
#define quantlib_western_calendar_h

#include "qldefines.h"
#include "calendar.h"

namespace QuantLib {

    namespace Calendars {

        //! Western calendar
        /*! This calendar provides derived calendars with the means of determining
            the Easter Monday for a given year.
        */
        class WesternCalendar : public Calendar {
          public:
            WesternCalendar() {}
          protected:
            //! expressed relative to first day of year
            static const Day easterMonday[];
        };

    }

}


#endif

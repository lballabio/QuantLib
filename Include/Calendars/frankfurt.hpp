
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

/*! \file frankfurt.hpp
    \brief Frankfurt calendar

    $Source$
    $Log$
    Revision 1.1  2001/04/04 11:07:21  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.6  2001/03/26 08:38:28  lballabio
    fixed line wrappings

    Revision 1.5  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.4  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_frankfurt_calendar_h
#define quantlib_frankfurt_calendar_h

#include "qldefines.hpp"
#include "westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Frankfurt calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Thursday</li>
            <li>Whit Monday</li>
            <li>Corpus Christi</li>
            <li>Labour Day, May 1st</li>
            <li>National Day, October 3rd</li>
            <li>Christmas Eve, December 24th</li>
            <li>Christmas, December 25th</li>
            <li>Boxing Day, December 26th</li>
            <li>New Year's Eve, December 31st</li>
            </ul>
        */
        class Frankfurt : public WesternCalendar {
          public:
            Frankfurt() {}
            std::string name() const { return "Frankfurt"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif


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

/*! \file helsinki.hpp
    \brief Helsinki calendar

    $Source$
    $Log$
    Revision 1.2  2001/04/04 12:13:22  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:21  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/26 09:59:35  lballabio
    Added Helsinki and Wellington calendars

*/

#ifndef quantlib_helsinki_calendar_h
#define quantlib_helsinki_calendar_h

#include "qldefines.hpp"
#include "Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Helsinki calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Epiphany, January 6th</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Thursday</li>
            <li>Labour Day, May 1st</li>
            <li>Midsummer Eve, June 21st</li>
            <li>Independence Day, December 6th</li>
            <li>Christmas Eve, December 24th</li>
            <li>Christmas, December 25th</li>
            <li>Boxing Day, December 26th</li>
            </ul>
            \note The holiday rules for Wellington were documented by 
            Veli-Pekka Mattila for IDB (http://www.jrefinery.com/ibd/)
        */
        class Helsinki : public WesternCalendar {
          public:
            Helsinki() {}
            std::string name() const { return "Helsinki"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif


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

/*! \file wellington.hpp
    \brief Wellington calendar

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

#ifndef quantlib_wellington_calendar_h
#define quantlib_wellington_calendar_h

#include "qldefines.hpp"
#include "Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Wellington calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday or 
                Tuesday)</li>
            <li>Day after New Year's Day, January 2st (possibly moved to 
                Monday or Tuesday)</li>
            <li>Anniversary Day, Monday nearest January 22nd</li>
            <li>Waitangi Day. February 6th</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>ANZAC Day. April 25th</li>
            <li>Queen's Birthday, first Monday in June</li>
            <li>Labour Day, fourth Monday in October</li>
            <li>Christmas, December 25th (possibly moved to Monday or 
                Tuesday)</li>
            <li>Boxing Day, December 26th (possibly moved to Monday or 
                Tuesday)</li>
            </ul>
            \note The holiday rules for Wellington were documented by 
            David Gilbert for IDB (http://www.jrefinery.com/ibd/)
        */
        class Wellington : public WesternCalendar {
          public:
            Wellington() {}
            std::string name() const { return "Wellington"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif

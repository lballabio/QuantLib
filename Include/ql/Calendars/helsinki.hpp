
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

/*! \file helsinki.hpp
    \fullpath Include/ql/Calendars/%helsinki.hpp
    \brief Helsinki calendar

*/

// $Id$
// $Log$
// Revision 1.7  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.6  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_helsinki_calendar_h
#define quantlib_helsinki_calendar_h

#include "ql/Calendars/westerncalendar.hpp"

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

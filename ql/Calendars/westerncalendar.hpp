
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

/*! \file westerncalendar.hpp

    \fullpath
    Include/ql/Calendars/%westerncalendar.hpp
    \brief Western calendar

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:02:31  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.8  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
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

#ifndef quantlib_western_calendar_h
#define quantlib_western_calendar_h

#include "ql/calendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! Western calendar
        /*! This calendar provides derived calendars with the means of
            determining the Easter Monday for a given year.
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


/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:03:55  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file milan.hpp
    \brief Milan calendar
*/

#ifndef quantlib_milan_calendar_h
#define quantlib_milan_calendar_h

#include "ql/Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Milan calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Epiphany, January 6th</li>
            <li>Easter Monday</li>
            <li>Liberation Day, April 25th</li>
            <li>Labour Day, May 1st</li>
            <li>Assumption, August 15th</li>
            <li>All Saint's Day, November 1st</li>
            <li>Immaculate Conception, December 8th</li>
            <li>Christmas, December 25th</li>
            <li>St. Stephen, December 26th</li>
            </ul>
        */
        class Milan : public WesternCalendar {
          public:
            Milan() {}
            std::string name() const { return "Milan"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif


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
    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file newyork.hpp
    \brief New York calendar
*/

#ifndef quantlib_newyork_calendar_h
#define quantlib_newyork_calendar_h

#include "qldefines.hpp"
#include "Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! New York calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday if
                actually on Sunday, or to Friday if on Saturday)</li>
            <li>Martin Luther King's birthday, third Monday in January</li>
            <li>Washington's birthday, third Monday in February</li>
            <li>Memorial Day, last Monday in May</li>
            <li>Independence Day, July 4th (moved to Monday if Sunday or Friday
                if Saturday)</li>
            <li>Labor Day, first Monday in September</li>
            <li>Columbus Day, second Monday in October</li>
            <li>Veteran's Day, November 11th (moved to Monday if Sunday or
                Friday if Saturday)</li>
            <li>Thanksgiving Day, fourth Thursday in November</li>
            <li>Christmas, December 25th (moved to Monday if Sunday or Friday if
                Saturday)</li>
            </ul>
        */
        class NewYork : public WesternCalendar {
          public:
            NewYork() {}
            std::string name() const { return "NewYork"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif

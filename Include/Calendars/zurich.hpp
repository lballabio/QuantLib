
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

/*! \file zurich.hpp
    \brief Zurich calendar
*/

#ifndef quantlib_zurich_calendar_h
#define quantlib_zurich_calendar_h

#include "qldefines.hpp"
#include "Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! %Zurich calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st</li>
            <li>Berchtoldstag, January 2nd</li>
            <li>Good Friday</li>
            <li>Easter Monday</li>
            <li>Ascension Day</li>
            <li>Whit Monday</li>
            <li>Labour Day, May 1st</li>
            <li>National Day, August 1st</li>
            <li>Christmas, December 25th</li>
            <li>St. Stephen's Day, December 26th</li>
            </ul>
        */
        class Zurich : public WesternCalendar {
          public:
            Zurich() {}
            std::string name() const { return "Zurich"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif

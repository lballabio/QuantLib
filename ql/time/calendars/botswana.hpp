/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
Copyright (C) 2017 Francois Botha

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file botswana.hpp
\brief Botswana calendar
*/

#ifndef quantlib_botswana_calendar_hpp
#define quantlib_botswana_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Botswana calendar
    /*! Holidays:
    From the Botswana <a href="http://www.ilo.org/dyn/travail/docs/1766/Public%20Holidays%20Act.pdf">Public Holidays Act</a>    
    The days named in the Schedule shall be public holidays within Botswana:
    Provided that
    <ul>
    <li>when any of the said days fall on a Sunday the following Monday shall be observed as a public holiday;</li>
    <li>if 2nd January, 1st October or Boxing Day falls on a Monday, the following Tuesday shall be observed as a public holiday;</li>
    <li>when Botswana Day referred to in the Schedule falls on a Saturday, the next following Monday shall be observed as a public holiday.</li>
    </ul>
    <ul>
    <li>Saturdays</li>
    <li>Sundays</li>
    <li>New Year's Day, January 1st</li>
    <li>Good Friday</li>
    <li>Easter Monday</li>
    <li>Labour Day, May 1st</li>
    <li>Ascension</li>
    <li>Sir Seretse Khama Day, July 1st</li>
    <li>Presidents' Day</li>
    <li>Independence Day, September 30th</li>
    <li>Botswana Day, October 1st</li>
    <li>Christmas, December 25th </li>
    <li>Boxing Day, December 26th</li>
    </ul>

    \ingroup calendars
    */
    class Botswana : public Calendar {
    private:
        class Impl final : public Calendar::WesternImpl {
        public:
          std::string name() const override { return "Botswana"; }
          bool isBusinessDay(const Date&) const override;
        };
    public:
        Botswana();
    };

}


#endif

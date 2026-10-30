/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 SoftSolution srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file serbia.hpp
    \brief Serbia calendars
*/

#ifndef quantlib_serbia_calendar_hpp
#define quantlib_serbia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Serbian calendars
    /*! Holidays for the Belgrade stock exchange (see https://www.belex.rs/eng/trzista_i_hartije/kalendar):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year, January 1st</li>
        <li>New Year, January 2nd</li>
        <li>Serbian Orthodox Christmas, January 7th</li>
        <li>Serbian Statehood Day, February 15 & 16th</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Armistice Day in World War I, Novemeber 11th</li>
        <li>Trading system maintenance, statistics and data migration, December 31st</li>
        </ul>
        \ingroup calendars
    */
    class Serbia : public Calendar {
    private:
        class BseImpl final : public Calendar::WesternImpl {
        public:
            std::string name() const override { return "Belgrade stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
    public:
        enum Market {
            BSE    //!< Belgrade stock exchange
        };
        explicit Serbia(Market m = BSE);
    };

}


#endif

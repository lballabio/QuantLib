/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011 StatPro Italia srl

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

/*! \file indonesia.hpp
    \brief Indonesian calendars
*/

#ifndef quantlib_indonesia_calendar_hpp
#define quantlib_indonesia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! %Indonesian calendars
    /*! Holidays for the Indonesia stock exchange
        (data from <http://www.idx.co.id/>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Ascension of Jesus Christ</li>
        <li>Independence Day, August 17th</li>
        <li>Christmas, December 25th</li>
        </ul>

        Other holidays for which no rule is given
        (data available for 2005-2014 only:)
        <ul>
        <li>Idul Adha</li>
        <li>Ied Adha</li>
        <li>Imlek</li>
        <li>Moslem's New Year Day</li>
        <li>Chinese New Year</li>
        <li>Nyepi (Saka's New Year)</li>
        <li>Birthday of Prophet Muhammad SAW</li>
        <li>Waisak</li>
        <li>Ascension of Prophet Muhammad SAW</li>
        <li>Idul Fitri</li>
        <li>Ied Fitri</li>
        <li>Other national leaves</li>
        </ul>
        \ingroup calendars
    */
    class Indonesia : public Calendar {
      private:
        class BejImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Jakarta stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        enum Market { BEJ,  //!< Jakarta stock exchange (merged into IDX)
                      JSX,  //!< Jakarta stock exchange (merged into IDX)
                      IDX   //!< Indonesia stock exchange
        };
        Indonesia(Market m = IDX);
    };

}


#endif


#ifndef id_7ce59d6f349c103a832cad48c931d0d4
#define id_7ce59d6f349c103a832cad48c931d0d4
inline bool test_7ce59d6f349c103a832cad48c931d0d4(const int* i) {
    return i != nullptr;
}
#endif

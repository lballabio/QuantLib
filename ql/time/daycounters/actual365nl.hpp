/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 BGC Partners L.P.

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

/*! \file actual365nl.hpp
    \brief Actual/365 (No Leap) day counter
*/

#ifndef quantlib_actual365nl_h
#define quantlib_actual365nl_h

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Actual/365 (No Leap) day count convention
    /*! "Actual/365 (No Leap)" day count convention, also known as
        "Act/365 (NL)", "NL/365", or "Actual/365 (JGB)".

        \ingroup daycounters
    */
    class Actual365NoLeap : public DayCounter {
    private:
        class Impl : public DayCounter::Impl {
        public:
            std::string name() const { return std::string("Actual/365 (NL)"); }

            // Returns the exact number of days between 2 dates, excluding leap days
            BigInteger dayCount(const Date& d1,
                                const Date& d2) const {

                static const Integer MonthOffset[] = {
                    0,  31,  59,  90, 120, 151,  // Jan - Jun
                  181, 212, 243, 273, 304, 334   // Jun - Dec
                };
                BigInteger s1, s2;

                s1 = d1.dayOfMonth() + MonthOffset[d1.month()-1] + (d1.year() * 365);
                s2 = d2.dayOfMonth() + MonthOffset[d2.month()-1] + (d2.year() * 365);

                if (d1.month() == Feb && d1.dayOfMonth() == 29)
                {
                    --s1;
                }

                if (d2.month() == Feb && d2.dayOfMonth() == 29)
                {
                    --s2;
                }

                return s2 - s1;
            }

            QuantLib::Time yearFraction(const Date& d1,
                                        const Date& d2,
                                        const Date& d3,
                                        const Date& d4) const {
                return dayCount(d1, d2)/365.0;
            }
        };
    public:
        Actual365NoLeap()
        : DayCounter(boost::shared_ptr<DayCounter::Impl>(
                                                new Actual365NoLeap::Impl)) {}
    };

}

#endif


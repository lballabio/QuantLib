/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/time/daycounters/thirty360.hpp>

namespace QuantLib {

    namespace { DayCounter fallback = Thirty360(Thirty360::BondBasis); }

    Date::serial_type SimpleDayCounter::Impl::dayCount(const Date& d1,
                                                       const Date& d2) const {
        return fallback.dayCount(d1,d2);
    }

    Time SimpleDayCounter::Impl::yearFraction(const Date& d1,
                                              const Date& d2,
                                              const Date&,
                                              const Date&) const {
        Day dm1 = d1.dayOfMonth(),
            dm2 = d2.dayOfMonth();

        if (dm1 == dm2 ||
            // e.g., Aug 30 -> Feb 28 ?
            (dm1 > dm2 && Date::isEndOfMonth(d2)) ||
            // e.g., Feb 28 -> Aug 30 ?
            (dm1 < dm2 && Date::isEndOfMonth(d1))) {

            return (d2.year()-d1.year()) +
                (Integer(d2.month())-Integer(d1.month()))/12.0;

        } else {
            return fallback.yearFraction(d1,d2);
        }
    }

}


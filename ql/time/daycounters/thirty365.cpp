/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 StatPro Italia srl

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

#include <ql/time/daycounters/thirty365.hpp>

namespace QuantLib {

    Date::serial_type Thirty365::Impl::dayCount(const Date& d1,
                                                const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Integer mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        return 360*(yy2-yy1) + 30*(mm2-mm1) + (dd2-dd1);
    }

    Thirty365::Thirty365()
    : DayCounter(ext::shared_ptr<DayCounter::Impl>(new Thirty365::Impl)) {}

}

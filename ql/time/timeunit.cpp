/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/time/timeunit.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    // timeunit formatting

    std::ostream& operator<<(std::ostream& out, const TimeUnit& timeunit) {
        switch (timeunit) {
            case Years:
                return out << "Years";
            case Months:
                return out << "Months";
            case Weeks:
                return out << "Weeks";
            case Days:
                return out << "Days";
            case Hours:
                return out << "Hours";
            case Minutes:
                return out << "Minutes";
            case Seconds:
                return out << "Seconds";
            case Milliseconds:
                return out << "Milliseconds";
            case Microseconds:
                return out << "Microseconds";
            default:
                QL_FAIL("unknown TimeUnit");
        }
    }

}

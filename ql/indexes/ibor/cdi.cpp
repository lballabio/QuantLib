/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 StatPro Italia srl

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

#include <ql/indexes/ibor/cdi.hpp>

namespace QuantLib {

    Rate Cdi::forecastFixing(const Date& fixingDate) const {

        QL_REQUIRE(!termStructure_.empty(),
                   "null term structure set to this instance of " << name());

        const Date d1 = valueDate(fixingDate);
        const Date d2 = maturityDate(d1);
        const Time t = dayCounter_.yearFraction(d1, d2);
        QL_REQUIRE(t > 0.0, "\n cannot calculate forward rate between "
                                << d1 << " and " << d2 << ":\n non positive time (" << t
                                << ") using " << dayCounter_.name() << " daycounter");
        
        //const DiscountFactor disc1 = termStructure_->discount(d1);
        //const DiscountFactor disc2 = termStructure_->discount(d2);
        //return std::pow(disc1 / disc2, 1.0 / t) - 1.0;

        return termStructure_->forwardRate(d1, d2, dayCounter_, Compounded, Annual).rate();
    }
}

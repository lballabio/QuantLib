/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl

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

#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <utility>

namespace QuantLib {

    FlatForward::FlatForward(const Date& referenceDate,
                             Handle<Quote> forward,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter), forward_(std::move(forward)),
      compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
    }

    FlatForward::FlatForward(const Date& referenceDate,
                             Rate forward,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter),
      forward_(ext::shared_ptr<Quote>(new SimpleQuote(forward))),
      compounding_(compounding), frequency_(frequency) {}

    FlatForward::FlatForward(Natural settlementDays,
                             const Calendar& calendar,
                             Handle<Quote> forward,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency)
    : YieldTermStructure(settlementDays, calendar, dayCounter), forward_(std::move(forward)),
      compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
    }

    FlatForward::FlatForward(Natural settlementDays,
                             const Calendar& calendar,
                             Rate forward,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency)
    : YieldTermStructure(settlementDays, calendar, dayCounter),
      forward_(ext::shared_ptr<Quote>(new SimpleQuote(forward))),
      compounding_(compounding), frequency_(frequency) {}

}

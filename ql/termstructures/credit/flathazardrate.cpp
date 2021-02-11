/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <utility>

namespace QuantLib {

    FlatHazardRate::FlatHazardRate(const Date& referenceDate,
                                   Handle<Quote> hazardRate,
                                   const DayCounter& dayCounter)
    : HazardRateStructure(referenceDate, Calendar(), dayCounter),
      hazardRate_(std::move(hazardRate)) {
        registerWith(hazardRate_);
    }

    FlatHazardRate::FlatHazardRate(const Date& referenceDate,
                                   Rate hazardRate,
                                   const DayCounter& dayCounter)
    : HazardRateStructure(referenceDate, Calendar(), dayCounter),
      hazardRate_(ext::shared_ptr<Quote>(new SimpleQuote(hazardRate))) {}

    FlatHazardRate::FlatHazardRate(Natural settlementDays,
                                   const Calendar& calendar,
                                   Handle<Quote> hazardRate,
                                   const DayCounter& dayCounter)
    : HazardRateStructure(settlementDays, calendar, dayCounter),
      hazardRate_(std::move(hazardRate)) {
        registerWith(hazardRate_);
    }

    FlatHazardRate::FlatHazardRate(Natural settlementDays,
                                   const Calendar& calendar,
                                   Rate hazardRate,
                                   const DayCounter& dayCounter)
    : HazardRateStructure(settlementDays, calendar, dayCounter),
      hazardRate_(ext::shared_ptr<Quote>(new SimpleQuote(hazardRate))) {}

}

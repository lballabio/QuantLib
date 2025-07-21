/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/math/comparison.hpp>
#include <ql/termstructure.hpp>
#include <utility>

namespace QuantLib {

    TermStructure::TermStructure(DayCounter dc)
    : settlementDays_(Null<Natural>()), dayCounter_(std::move(dc)) {}

    TermStructure::TermStructure(const Date& referenceDate, Calendar cal, DayCounter dc)
    : calendar_(std::move(cal)), referenceDate_(referenceDate), settlementDays_(Null<Natural>()),
      dayCounter_(std::move(dc)) {}

    TermStructure::TermStructure(Natural settlementDays, Calendar cal, DayCounter dc)
    : moving_(true), updated_(false), calendar_(std::move(cal)), settlementDays_(settlementDays),
      dayCounter_(std::move(dc)) {
        registerWith(Settings::instance().evaluationDate());
    }

    const Date& TermStructure::referenceDate() const {
        if (!updated_) {
            Date today = Settings::instance().evaluationDate();
            referenceDate_ = calendar().advance(today, settlementDays(), Days);
            updated_ = true;
        }
        return referenceDate_;
    }

    void TermStructure::update() {
        if (moving_)
            updated_ = false;
        notifyObservers();
    }

    void TermStructure::checkRange(const Date& d,
                                   bool extrapolate) const {
        QL_REQUIRE(d >= referenceDate(),
                   "date (" << d << ") before reference date (" <<
                   referenceDate() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || d <= maxDate(),
                   "date (" << d << ") is past max curve date ("
                            << maxDate() << ")");
    }

    void TermStructure::checkRange(Time t,
                                   bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation()
                   || t <= maxTime() || close_enough(t, maxTime()),
                   "time (" << t << ") is past max curve time ("
                            << maxTime() << ")");
    }

}

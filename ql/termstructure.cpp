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

#include <ql/termstructure.hpp>

namespace QuantLib {

    TermStructure::TermStructure(const DayCounter& dc)
    : moving_(false),
      updated_(true),
      settlementDays_(Null<Size>()),
      dayCounter_(dc) {}

    TermStructure::TermStructure(const Date& referenceDate,
                                 const Calendar& cal,
                                 const DayCounter& dc)
    : moving_(false),
      referenceDate_(referenceDate), updated_(true),
      settlementDays_(Null<Natural>()), calendar_(cal),
      dayCounter_(dc) {}

    TermStructure::TermStructure(Natural settlementDays,
                                 const Calendar& cal,
                                 const DayCounter& dc)
    : moving_(true),
      updated_(false),
      settlementDays_(settlementDays), calendar_(cal),
      dayCounter_(dc) {
        registerWith(Settings::instance().evaluationDate());
        // verify immediately if calendar and settlementDays are ok
        Date today = Settings::instance().evaluationDate();
        referenceDate_ = calendar().advance(today, settlementDays_, Days);
    }

    const Date& TermStructure::referenceDate() const {
        if (!updated_) {
            Date today = Settings::instance().evaluationDate();
            referenceDate_ = calendar().advance(today, settlementDays_, Days);
            updated_ = true;
        }
        return referenceDate_;
    }

    void TermStructure::update() {
        if (moving_)
            updated_ = false;
        notifyObservers();
    }

    void TermStructure::checkRange(Time t, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                            << maxTime() << ")");
    }

}

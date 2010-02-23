/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    void SmileSection::update() {
        if (isFloating_) {
            referenceDate_ = Settings::instance().evaluationDate();
            initializeExerciseTime();
        }
    }

    void SmileSection::initializeExerciseTime() const {
        QL_REQUIRE(exerciseDate_>=referenceDate_,
                   "expiry date (" << exerciseDate_ <<
                   ") must be greater than reference date (" <<
                   referenceDate_ << ")");
        exerciseTime_ = dc_.yearFraction(referenceDate_, exerciseDate_);
    }

    SmileSection::SmileSection(const Date& d,
                               const DayCounter& dc,
                               const Date& referenceDate)
    : exerciseDate_(d), dc_(dc) {
        isFloating_ = referenceDate==Date();
        if (isFloating_) {
            registerWith(Settings::instance().evaluationDate());
            referenceDate_ = Settings::instance().evaluationDate();
        } else
            referenceDate_ = referenceDate;
        initializeExerciseTime();
    }

    SmileSection::SmileSection(Time exerciseTime,
                               const DayCounter& dc)
    : isFloating_(false), referenceDate_(Date()),
      dc_(dc), exerciseTime_(exerciseTime) {
        QL_REQUIRE(exerciseTime_>=0.0,
                   "expiry time must be positive: " <<
                   exerciseTime_ << " not allowed");
    }

}

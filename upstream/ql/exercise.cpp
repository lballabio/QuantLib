/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/exercise.hpp>
#include <ql/errors.hpp>
#include <algorithm>

namespace QuantLib {

    Date Exercise::lastDate() const {
        QL_REQUIRE(!dates_.empty(), "no exercise date given");
        return dates_.back();
    }

    AmericanExercise::AmericanExercise(const Date& earliest,
                                       const Date& latest,
                                       bool payoffAtExpiry)
    : EarlyExercise(American, payoffAtExpiry) {
        QL_REQUIRE(earliest<=latest,
                   "earliest > latest exercise date");
        dates_ = std::vector<Date>(2);
        dates_[0] = earliest;
        dates_[1] = latest;
    }

    AmericanExercise::AmericanExercise(const Date& latest,
                                       bool payoffAtExpiry)
    : EarlyExercise(American, payoffAtExpiry) {
        dates_ = std::vector<Date>(2);
        dates_[0] = Date::minDate();
        dates_[1] = latest;
    }

    BermudanExercise::BermudanExercise(const std::vector<Date>& dates,
                                       bool payoffAtExpiry)
    : EarlyExercise(Bermudan, payoffAtExpiry) {
        QL_REQUIRE(!dates.empty(), "no exercise date given");
        dates_ = dates;
        std::sort(dates_.begin(), dates_.end());
    }

    EuropeanExercise::EuropeanExercise(const Date& date)
    : Exercise(European) {
        dates_ = std::vector<Date>(1,date);
    }

}

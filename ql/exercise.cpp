
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>

namespace QuantLib {

    AmericanExercise::AmericanExercise(Date earliest, Date latest,
        bool payoffAtExpiry)
    : EarlyExercise(payoffAtExpiry) {

        // American exercise cannot degenerate into European exercise
        QL_REQUIRE(earliest<latest,
            "AmericanExercise::AmericanExercise : "
            "earliest>=latest exercise date");
        type_ = American;
        dates_ = std::vector<Date>(2);
        dates_[0] = earliest;
        dates_[1] = latest;
    }

    BermudanExercise::BermudanExercise(const std::vector<Date>& dates,
        bool payoffAtExpiry)
    : EarlyExercise(payoffAtExpiry) {

        type_ = American;
        // Bermudan exercise cannot degenerate into European exercise
        QL_REQUIRE(dates.size()>1,
            "BermudanExercise::BermudanExercise : "
            "at least 2 exercise dates are needed");
        dates_ = dates;
        std::sort(dates_.begin(), dates_.end());

    }

    EuropeanExercise::EuropeanExercise(Date date)
    : Exercise() {
        type_ = European;
        dates_ = std::vector<Date>(1,date);
    }



}

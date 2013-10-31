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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

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

    RebatedExercise::RebatedExercise(
        const Exercise &exercise, const Real rebate,
        const Natural rebateSettlementDays,
        const Calendar &rebatePaymentCalendar,
        const BusinessDayConvention rebatePaymentConvention)
        : Exercise(exercise),
          rebates_(std::vector<Real>(dates().size(), rebate)),
          rebateSettlementDays_(rebateSettlementDays),
          rebatePaymentCalendar_(rebatePaymentCalendar),
          rebatePaymentConvention_(rebatePaymentConvention) {}

    RebatedExercise::RebatedExercise(
        const Exercise &exercise, const std::vector<Real> &rebates,
        const Natural rebateSettlementDays,
        const Calendar &rebatePaymentCalendar,
        const BusinessDayConvention rebatePaymentConvention)
        : Exercise(exercise), rebates_(rebates),
          rebateSettlementDays_(rebateSettlementDays),
          rebatePaymentCalendar_(rebatePaymentCalendar),
          rebatePaymentConvention_(rebatePaymentConvention) {

        QL_REQUIRE(
            type_ == Bermudan,
            "a rebate vector is allowed only for a bermudan style exercise");

        QL_REQUIRE(rebates.size() == dates().size(),
                   "the number of rebates ("
                       << rebates.size()
                       << ") must be equal to the number of exercise dates ("
                       << dates().size());
    }
}

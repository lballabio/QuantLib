/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/experimental/exercise/rebatedexercise.hpp>

namespace QuantLib {

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

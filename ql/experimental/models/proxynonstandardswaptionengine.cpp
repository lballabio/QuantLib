/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/models/proxynonstandardswaptionengine.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

void ProxyNonstandardSwaptionEngine::calculate() const {

    Date today = Settings::instance().evaluationDate();

    int offset = includeTodaysExercise_ ? -1 : 0;

    std::vector<Date>::const_iterator nextExerciseDate = std::upper_bound(
        proxy_->expiryDates.begin(), proxy_->expiryDates.end(), today + offset);

    // if no exercise date is left, the npv of the swaption is zero
    // a possibly generated physical swap is not valued here
    if (nextExerciseDate == proxy_->expiryDates.end()) {
        results_.value = 0.0;
        results_.additionalResults["exerciseValue"] = 0.0;
        return;
    }

    std::vector<Date>::const_iterator nextExerciseDateOrig =
        std::upper_bound(arguments_.exercise->dates().begin(),
                         arguments_.exercise->dates().end(), today);

    Size exerciseIdx = nextExerciseDate - proxy_->expiryDates.begin();

    Real todaysTime = proxy_->model->termStructure()->timeFromReference(today);

    Real nextExerciseTime =
        proxy_->model->termStructure()->timeFromReference(*nextExerciseDate);

    // sanity checks
    QL_REQUIRE(
        today >= proxy_->origEvalDate,
        "evaluation date ("
            << today
            << ") must be greater or equal than original evaluation date ("
            << proxy_->origEvalDate);

    StateHelper helper(&(*proxy_->model), referenceRate_->value(),
                       referenceMaturity_->value(), todaysTime);
    Brent b;
    Real todaysState;
    try {
        todaysState = b.solve(helper, 1E-4, 0.0, 0.0010);
    } catch (QuantLib::Error e) {
        QL_FAIL("could not imply model state (reference rate="
                << referenceRate_->value()
                << ", reference maturity=" << referenceMaturity_->value()
                << ", reference date=" << today << ")");
    }

    // we have to use the discount curve from the original pricing
    Handle<YieldTermStructure> discountTmp = proxy_->discount;

    // integrate over the exercise / continuation values on the next expiry date
    // if the next exercise date is the last one the continuation value is zero
    // if the next expiry date is equal to the valuation date we do not need to
    // integrate

    Array y(1, todaysState), z(1, 0.0), p(1, 0.0);

    if (!close(nextExerciseTime, todaysTime) && !integrationPoints_ == 0) {
        y = proxy_->model->yGrid(stdDevs_, integrationPoints_, nextExerciseTime,
                                 todaysTime, todaysState);
        z = proxy_->model->yGrid(stdDevs_, integrationPoints_);
        p = Array(z.size(), 0.0);
    }

    for (Size i = 0; i < y.size(); ++i) {

        // model state expressed in x
        Real nextExState = y[i] *
                               proxy_->model->stateProcess()->stdDeviation(
                                   0.0, 0.0, nextExerciseTime) +
                           proxy_->model->stateProcess()->expectation(
                               0.0, 0.0, nextExerciseTime);

        // continuation value (as per regression)
        p[i] = proxy_->regression[exerciseIdx]->operator()(nextExState);
    }

    Real zSpreadDf =
        proxy_->oas.empty()
            ? 1.0
            : std::exp(-proxy_->oas->value() * (nextExerciseTime - todaysTime));

    Real price = 0.0;
    if (y.size() == 1) {
        price = p[0];
    } else {
        CubicInterpolation payoff(z.begin(), z.end(), p.begin(),
                                  CubicInterpolation::Spline, true,
                                  CubicInterpolation::Lagrange, 0.0,
                                  CubicInterpolation::Lagrange, 0.0);
        for (Size i = 0; i < z.size() - 1; ++i) {
            price +=
                proxy_->model->gaussianShiftedPolynomialIntegral(
                    0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i],
                    payoff.aCoefficients()[i], p[i], z[i], z[i], z[i + 1]) *
                zSpreadDf;
        }
    }

    // discount back continuation value and exercise value

    Real df = proxy_->model->numeraire(today, todaysState, discountTmp);

    results_.value = df * price;
}

} // namespace QuantLib

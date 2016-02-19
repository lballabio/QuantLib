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

#include <ql/experimental/models/mcgaussian1dnonstandardswaptionengine.hpp>
#include <ql/rebatedexercise.hpp>

// #include <boost/math/special_functions/laguerre.hpp>

namespace QuantLib {

namespace {
Real basis0(const Real x) { return 1; }
Real basis1(const Real x) { return x; }
Real basis2(const Real x) { return x * x; }
}

Gaussian1dNonstandardSwaptionPathPricer::
    Gaussian1dNonstandardSwaptionPathPricer(
        const boost::shared_ptr<Gaussian1dModel> &model,
        const NonstandardSwaption::arguments *arguments,
        const Handle<YieldTermStructure> &discount, const Handle<Quote> &oas)
    : model_(model), arguments_(arguments), discount_(discount), oas_(oas) {

    // basis functions for ls regression
    basis_.push_back(boost::function1<Real, Real>(&basis0));
    basis_.push_back(boost::function1<Real, Real>(&basis1));
    basis_.push_back(boost::function1<Real, Real>(&basis2));

    // minimum alive exercise index
    Date today = Settings::instance().evaluationDate();
    minIdxAlive_ =
        std::upper_bound(arguments_->exercise->dates().begin(),
                         arguments_->exercise->dates().end(), today) -
        arguments_->exercise->dates().begin();
}

void Gaussian1dNonstandardSwaptionPathPricer::initExerciseIndices(
    const Path &path) const {
    // initialize the indices corresponding to the exercise dates
    if (exerciseIdx_.size() == 0) {
        for (Size i = 0, j = minIdxAlive_;
             i < path.length() && j < arguments_->exercise->dates().size();
             ++i) {
            if (close(
                    model_->stateProcess()->time(arguments_->exercise->date(j)),
                    path.time(i))) {
                exerciseIdx_.push_back(i);
                ++j;
            }
        }
        QL_REQUIRE(exerciseIdx_.size() ==
                       arguments_->exercise->dates().size() - minIdxAlive_,
                   "did not find all future exercise dates ("
                       << arguments_->exercise->dates().size() - minIdxAlive_
                       << ") in path times grid, only matched "
                       << exerciseIdx_.size() << " dates and grid times.");
    }
}

Real Gaussian1dNonstandardSwaptionPathPricer::state(const Path &path,
                                                    Size t) const {
    initExerciseIndices(path);
    return path[exerciseIdx_[t - 1]];
}

std::vector<boost::function1<Real, Real> >
Gaussian1dNonstandardSwaptionPathPricer::basisSystem() const {
    return basis_;
}

Real Gaussian1dNonstandardSwaptionPathPricer::operator()(const Path &path,
                                                         Size t) const {

    initExerciseIndices(path);

    // in the following we have to use a standardized state
    Real state = (path[exerciseIdx_[t - 1]] -
                  model_->stateProcess()->expectation(
                      0.0, 0.0, path.time(exerciseIdx_[t - 1]))) /
                 model_->stateProcess()->stdDeviation(
                     0.0, 0.0, path.time(exerciseIdx_[t - 1]));

    // price all cashflows that belong to the exercise into right
    // and return the deflated NPV
    boost::shared_ptr<RebatedExercise> rebatedExercise =
        boost::dynamic_pointer_cast<RebatedExercise>(arguments_->exercise);
    Date exDate = arguments_->exercise->date(minIdxAlive_ + (t - 1));
    boost::shared_ptr<NonstandardSwap> swap = arguments_->swap;
    Schedule fixedSchedule = swap->fixedSchedule();
    Schedule floatingSchedule = swap->floatingSchedule();
    Size j1 = std::upper_bound(fixedSchedule.dates().begin(),
                               fixedSchedule.dates().end(), exDate - 1) -
              fixedSchedule.dates().begin();
    Size k1 = std::upper_bound(floatingSchedule.dates().begin(),
                               floatingSchedule.dates().end(), exDate - 1) -
              floatingSchedule.dates().begin();

    // this is more or less copied from gaussian1dnonstandardswaptionengine.cpp
    Real floatingLegNpv = 0.0;
    for (Size l = k1; l < arguments_->floatingCoupons.size(); l++) {
        Real zSpreadDf =
            oas_.empty()
                ? 1.0
                : std::exp(-oas_->value() *
                           (model_->termStructure()->dayCounter().yearFraction(
                               exDate, arguments_->floatingPayDates[l])));
        Real amount;
        if (arguments_->floatingIsRedemptionFlow[l])
            amount = arguments_->floatingCoupons[l];
        else
            amount = arguments_->floatingNominal[l] *
                     arguments_->floatingAccrualTimes[l] *
                     (arguments_->floatingGearings[l] *
                          model_->forwardRate(
                              arguments_->floatingFixingDates[l], exDate, state,
                              arguments_->swap->iborIndex()) +
                      arguments_->floatingSpreads[l]);
        floatingLegNpv +=
            amount *
            model_->deflatedZerobond(arguments_->floatingPayDates[l], exDate,
                                     state, discount_, discount_) *
            zSpreadDf;
    }
    Real fixedLegNpv = 0.0;
    for (Size l = j1; l < arguments_->fixedCoupons.size(); l++) {
        Real zSpreadDf =
            oas_.empty()
                ? 1.0
                : std::exp(-oas_->value() *
                           (model_->termStructure()->dayCounter().yearFraction(
                               exDate, arguments_->fixedPayDates[l])));
        fixedLegNpv +=
            arguments_->fixedCoupons[l] *
            model_->deflatedZerobond(arguments_->fixedPayDates[l], exDate,
                                     state, discount_, discount_) *
            zSpreadDf;
    }
    Real rebate = 0.0;
    Real zSpreadDf = 1.0;
    Date rebateDate = exDate;
    if (rebatedExercise != NULL) {
        rebate = rebatedExercise->rebate(minIdxAlive_ + (t - 1));
        rebateDate = rebatedExercise->rebatePaymentDate(minIdxAlive_ + (t - 1));
        zSpreadDf =
            oas_.empty()
                ? 1.0
                : std::exp(-oas_->value() *
                           (model_->termStructure()->dayCounter().yearFraction(
                               exDate, rebateDate)));
    }
    Real exerciseValue =
        std::max(((arguments_->type == VanillaSwap::Payer ? 1.0 : -1.0) *
                      (floatingLegNpv - fixedLegNpv) +
                  rebate *
                      model_->deflatedZerobond(rebateDate, exDate, state,
                                               discount_, discount_) *
                      zSpreadDf),
                 0.0);

    return exerciseValue;
}

} // namespace QuantLib

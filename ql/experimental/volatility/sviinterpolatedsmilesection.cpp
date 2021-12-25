/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/experimental/volatility/sviinterpolatedsmilesection.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    SviInterpolatedSmileSection::SviInterpolatedSmileSection(
        const Date& optionDate,
        Handle<Quote> forward,
        const std::vector<Rate>& strikes,
        bool hasFloatingStrikes,
        Handle<Quote> atmVolatility,
        const std::vector<Handle<Quote> >& volHandles,
        Real a,
        Real b,
        Real sigma,
        Real rho,
        Real m,
        bool isAFixed,
        bool isBFixed,
        bool isSigmaFixed,
        bool isRhoFixed,
        bool isMFixed,
        bool vegaWeighted,
        ext::shared_ptr<EndCriteria> endCriteria,
        ext::shared_ptr<OptimizationMethod> method,
        const DayCounter& dc)
    : SmileSection(optionDate, dc), forward_(std::move(forward)),
      atmVolatility_(std::move(atmVolatility)), volHandles_(volHandles), strikes_(strikes),
      actualStrikes_(strikes), hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()),
      a_(a), b_(b), sigma_(sigma), rho_(rho), m_(m), isAFixed_(isAFixed), isBFixed_(isBFixed),
      isSigmaFixed_(isSigmaFixed), isRhoFixed_(isRhoFixed), isMFixed_(isMFixed),
      vegaWeighted_(vegaWeighted), endCriteria_(std::move(endCriteria)),
      method_(std::move(method)) {

        LazyObject::registerWith(forward_);
        LazyObject::registerWith(atmVolatility_);
        for (auto& volHandle : volHandles_)
            LazyObject::registerWith(volHandle);
    }

    SviInterpolatedSmileSection::SviInterpolatedSmileSection(
        const Date& optionDate,
        const Rate& forward,
        const std::vector<Rate>& strikes,
        bool hasFloatingStrikes,
        const Volatility& atmVolatility,
        const std::vector<Volatility>& volHandles,
        Real a,
        Real b,
        Real sigma,
        Real rho,
        Real m,
        bool isAFixed,
        bool isBFixed,
        bool isSigmaFixed,
        bool isRhoFixed,
        bool isMFixed,
        bool vegaWeighted,
        ext::shared_ptr<EndCriteria> endCriteria,
        ext::shared_ptr<OptimizationMethod> method,
        const DayCounter& dc)
    : SmileSection(optionDate, dc),
      forward_(Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(forward)))),
      atmVolatility_(Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(atmVolatility)))),
      volHandles_(volHandles.size()), strikes_(strikes), actualStrikes_(strikes),
      hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()), a_(a), b_(b),
      sigma_(sigma), rho_(rho), m_(m), isAFixed_(isAFixed), isBFixed_(isBFixed),
      isSigmaFixed_(isSigmaFixed), isRhoFixed_(isRhoFixed), isMFixed_(isMFixed),
      vegaWeighted_(vegaWeighted), endCriteria_(std::move(endCriteria)),
      method_(std::move(method)) {

        for (Size i = 0; i < volHandles_.size(); ++i)
            volHandles_[i] = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(volHandles[i])));
    }

void SviInterpolatedSmileSection::createInterpolation() const {
    ext::shared_ptr<SviInterpolation> tmp(new SviInterpolation(
        actualStrikes_.begin(), actualStrikes_.end(), vols_.begin(),
        exerciseTime(), forwardValue_, a_, b_, sigma_, rho_, m_, isAFixed_,
        isBFixed_, isSigmaFixed_, isRhoFixed_, isMFixed_, vegaWeighted_,
        endCriteria_, method_));
    swap(tmp, sviInterpolation_);
}

void SviInterpolatedSmileSection::performCalculations() const {
    forwardValue_ = forward_->value();
    vols_.clear();
    actualStrikes_.clear();
    // we populate the volatilities, skipping the invalid ones
    for (Size i = 0; i < volHandles_.size(); ++i) {
        if (volHandles_[i]->isValid()) {
            if (hasFloatingStrikes_) {
                actualStrikes_.push_back(forwardValue_ + strikes_[i]);
                vols_.push_back(atmVolatility_->value() +
                                volHandles_[i]->value());
            } else {
                actualStrikes_.push_back(strikes_[i]);
                vols_.push_back(volHandles_[i]->value());
            }
        }
    }
    // we are recreating the sabrinterpolation object unconditionnaly to
    // avoid iterator invalidation
    createInterpolation();
    sviInterpolation_->update();
}

Real SviInterpolatedSmileSection::varianceImpl(Real strike) const {
    calculate();
    Real v = (*sviInterpolation_)(strike, true);
    return v * v * exerciseTime();
}
}

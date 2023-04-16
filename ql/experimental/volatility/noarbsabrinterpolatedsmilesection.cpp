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

#include <ql/experimental/volatility/noarbsabrinterpolatedsmilesection.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    NoArbSabrInterpolatedSmileSection::NoArbSabrInterpolatedSmileSection(
        const Date& optionDate,
        Handle<Quote> forward,
        const std::vector<Rate>& strikes,
        bool hasFloatingStrikes,
        Handle<Quote> atmVolatility,
        const std::vector<Handle<Quote> >& volHandles,
        Real alpha,
        Real beta,
        Real nu,
        Real rho,
        bool isAlphaFixed,
        bool isBetaFixed,
        bool isNuFixed,
        bool isRhoFixed,
        bool vegaWeighted,
        std::shared_ptr<EndCriteria> endCriteria,
        std::shared_ptr<OptimizationMethod> method,
        const DayCounter& dc)
    : SmileSection(optionDate, dc), forward_(std::move(forward)),
      atmVolatility_(std::move(atmVolatility)), volHandles_(volHandles), strikes_(strikes),
      actualStrikes_(strikes), hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()),
      alpha_(alpha), beta_(beta), nu_(nu), rho_(rho), isAlphaFixed_(isAlphaFixed),
      isBetaFixed_(isBetaFixed), isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed),
      vegaWeighted_(vegaWeighted), endCriteria_(std::move(endCriteria)),
      method_(std::move(method)) {

        LazyObject::registerWith(forward_);
        LazyObject::registerWith(atmVolatility_);
        for (auto& volHandle : volHandles_)
            LazyObject::registerWith(volHandle);
    }

    NoArbSabrInterpolatedSmileSection::NoArbSabrInterpolatedSmileSection(
        const Date& optionDate,
        const Rate& forward,
        const std::vector<Rate>& strikes,
        bool hasFloatingStrikes,
        const Volatility& atmVolatility,
        const std::vector<Volatility>& volHandles,
        Real alpha,
        Real beta,
        Real nu,
        Real rho,
        bool isAlphaFixed,
        bool isBetaFixed,
        bool isNuFixed,
        bool isRhoFixed,
        bool vegaWeighted,
        std::shared_ptr<EndCriteria> endCriteria,
        std::shared_ptr<OptimizationMethod> method,
        const DayCounter& dc)
    : SmileSection(optionDate, dc),
      forward_(Handle<Quote>(std::shared_ptr<Quote>(new SimpleQuote(forward)))),
      atmVolatility_(Handle<Quote>(std::shared_ptr<Quote>(new SimpleQuote(atmVolatility)))),
      volHandles_(volHandles.size()), strikes_(strikes), actualStrikes_(strikes),
      hasFloatingStrikes_(hasFloatingStrikes), vols_(volHandles.size()), alpha_(alpha), beta_(beta),
      nu_(nu), rho_(rho), isAlphaFixed_(isAlphaFixed), isBetaFixed_(isBetaFixed),
      isNuFixed_(isNuFixed), isRhoFixed_(isRhoFixed), vegaWeighted_(vegaWeighted),
      endCriteria_(std::move(endCriteria)), method_(std::move(method)) {

        for (Size i = 0; i < volHandles_.size(); ++i)
            volHandles_[i] = Handle<Quote>(std::shared_ptr<Quote>(new SimpleQuote(volHandles[i])));
    }

    void NoArbSabrInterpolatedSmileSection::createInterpolation() const {
         std::shared_ptr<NoArbSabrInterpolation> tmp(new NoArbSabrInterpolation(
                     actualStrikes_.begin(), actualStrikes_.end(), vols_.begin(),
                     exerciseTime(), forwardValue_,
                     alpha_, beta_, nu_, rho_,
                     isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_, vegaWeighted_,
                     endCriteria_, method_));
         swap(tmp, noArbSabrInterpolation_);
    }

    void NoArbSabrInterpolatedSmileSection::performCalculations() const {
        forwardValue_ = forward_->value();
        vols_.clear();
        actualStrikes_.clear();
        // we populate the volatilities, skipping the invalid ones
        for (Size i=0; i<volHandles_.size(); ++i) {
            if (volHandles_[i]->isValid()) {
                if (hasFloatingStrikes_) {
                    actualStrikes_.push_back(forwardValue_ + strikes_[i]);
                    vols_.push_back(atmVolatility_->value() + volHandles_[i]->value());
                } else {
                    actualStrikes_.push_back(strikes_[i]);
                    vols_.push_back(volHandles_[i]->value());
                }
            }
        }
        // we are recreating the sabrinterpolation object unconditionnaly to
        // avoid iterator invalidation
        createInterpolation();
        noArbSabrInterpolation_->update();
    }

    Real NoArbSabrInterpolatedSmileSection::varianceImpl(Real strike) const {
        calculate();
        Real v = (*noArbSabrInterpolation_)(strike, true);
        return v*v*exerciseTime();
    }

}


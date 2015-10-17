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

#include <ql/experimental/models/lgm1.hpp>
#include <ql/quotes/simplequote.hpp>

#include <boost/make_shared.hpp>

namespace QuantLib {

Lgm1::Lgm1(const Handle<YieldTermStructure> &yts,
           const std::vector<Date> &volstepdates,
           const std::vector<Real> &alpha, const Real &kappa)
    : Lgm(yts), CalibratedModel(2), volstepdates_(volstepdates),
      volsteptimes_(volstepdates_.size()),
      volsteptimesArray_(volstepdates_.size()), alpha_(arguments_[0]),
      kappa_(arguments_[1]) {
    alphaQuotes_.resize(alpha.size());
    for (Size i = 0; i < alpha.size(); ++i) {
        alphaQuotes_[i] =
            Handle<Quote>(boost::make_shared<SimpleQuote>(alpha[i]));
    }

    kappaQuote_ = Handle<Quote>(boost::make_shared<SimpleQuote>(kappa));

    initialize();
}

Lgm1::Lgm1(const Handle<YieldTermStructure> &yts,
           const std::vector<Date> &volstepdates,
           const std::vector<Handle<Quote> > &alpha, const Handle<Quote> &kappa)
    : Lgm(yts), CalibratedModel(2), volstepdates_(volstepdates),
      volsteptimes_(volstepdates_.size()),
      volsteptimesArray_(volstepdates_.size()), alphaQuotes_(alpha),
      kappaQuote_(kappa), alpha_(arguments_[0]), kappa_(arguments_[1]) {

    initialize();
}

void Lgm1::updateTimes() const {
    volsteptimes_.clear();
    int j = 0;
    for (std::vector<Date>::const_iterator i = volstepdates_.begin();
         i != volstepdates_.end(); ++i, ++j) {
        volsteptimes_.push_back(termStructure()->timeFromReference(*i));
        volsteptimesArray_[j] = volsteptimes_[j];
        if (j == 0)
            QL_REQUIRE(volsteptimes_[0] > 0.0, "volsteptimes must be positive ("
                                                   << volsteptimes_[0] << ")");
        else
            QL_REQUIRE(volsteptimes_[j] > volsteptimes_[j - 1],
                       "volsteptimes must be strictly increasing ("
                           << volsteptimes_[j - 1] << "@" << (j - 1) << ", "
                           << volsteptimes_[j] << "@" << j << ")");
    }
}

void Lgm1::updateAlpha() {
    for (Size i = 0; i < alpha_.size(); ++i) {
        alpha_.setParam(i, alphaQuotes_[i]->value());
    }
    update();
}

void Lgm1::updateKappa() {
    kappa_.setParam(0, kappaQuote_->value());
    update();
}

void Lgm1::initialize() {
    QL_REQUIRE(volstepdates_.size() + 1 == alphaQuotes_.size(),
               "alphas (" << alphaQuotes_.size() << ") and step dates ("
                          << volstepdates_.size() << ") inconsistent.");
    updateTimes();
    alpha_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
    kappa_ = ConstantParameter(kappaQuote_->value(), NoConstraint());
    updateAlpha();
    alphaObserver_ = boost::make_shared<AlphaObserver>(this);
    kappaObserver_ = boost::make_shared<KappaObserver>(this);
    for (Size i = 0; i < alpha_.size(); ++i)
        alphaObserver_->registerWith(alphaQuotes_[i]);
    kappaObserver_->registerWith(kappaQuote_);
    setParametrization(
        boost::make_shared<detail::LgmPiecewiseAlphaConstantKappa>(
            volsteptimesArray_, alpha_.params(), kappa_.params()));
    stateProcess_ = boost::make_shared<
        LgmStateProcess<detail::LgmPiecewiseAlphaConstantKappa> >(
        parametrization());
    registerWith(stateProcess_);
    parametrization()->update();
}

} // namespace QuantLib

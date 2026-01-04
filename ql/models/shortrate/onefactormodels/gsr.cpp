/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/quotes/simplequote.hpp>
#include <utility>

namespace QuantLib {

    Gsr::Gsr(const Handle<YieldTermStructure>& termStructure,
             std::vector<Date> volstepdates,
             const std::vector<Real>& volatilities,
             const Real reversion,
             const Real T)
    : Gaussian1dModel(termStructure), CalibratedModel(2), reversion_(arguments_[0]),
      sigma_(arguments_[1]), volstepdates_(std::move(volstepdates)) {

        QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");

        volatilities_.resize(volatilities.size());
        for (Size i = 0; i < volatilities.size(); ++i)
            volatilities_[i] = Handle<Quote>(ext::make_shared<SimpleQuote>(volatilities[i]));
        reversions_.resize(1);
        reversions_[0] = Handle<Quote>(ext::make_shared<SimpleQuote>(reversion));

        initialize(T);
    }

    Gsr::Gsr(const Handle<YieldTermStructure>& termStructure,
             std::vector<Date> volstepdates,
             const std::vector<Real>& volatilities,
             const std::vector<Real>& reversions,
             const Real T)
    : Gaussian1dModel(termStructure), CalibratedModel(2), reversion_(arguments_[0]),
      sigma_(arguments_[1]), volstepdates_(std::move(volstepdates)) {

        QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");

        volatilities_.resize(volatilities.size());
        for (Size i = 0; i < volatilities.size(); ++i)
            volatilities_[i] = Handle<Quote>(ext::make_shared<SimpleQuote>(volatilities[i]));
        reversions_.resize(reversions.size());
        for (Size i = 0; i < reversions.size(); ++i)
            reversions_[i] = Handle<Quote>(ext::make_shared<SimpleQuote>(reversions[i]));

        initialize(T);
    }

    Gsr::Gsr(const Handle<YieldTermStructure>& termStructure,
             std::vector<Date> volstepdates,
             std::vector<Handle<Quote> > volatilities,
             const Handle<Quote>& reversion,
             const Real T)
    : Gaussian1dModel(termStructure), CalibratedModel(2), reversion_(arguments_[0]),
      sigma_(arguments_[1]), volatilities_(std::move(volatilities)),
      reversions_(std::vector<Handle<Quote> >(1, reversion)),
      volstepdates_(std::move(volstepdates)) {

        QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");
        initialize(T);
    }

    Gsr::Gsr(const Handle<YieldTermStructure>& termStructure,
             std::vector<Date> volstepdates,
             std::vector<Handle<Quote> > volatilities,
             std::vector<Handle<Quote> > reversions,
             const Real T)
    : Gaussian1dModel(termStructure), CalibratedModel(2), reversion_(arguments_[0]),
      sigma_(arguments_[1]), volatilities_(std::move(volatilities)),
      reversions_(std::move(reversions)), volstepdates_(std::move(volstepdates)) {

        QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");
        initialize(T);
    }

void Gsr::update() {
    if (stateProcess_ != nullptr)
        ext::static_pointer_cast<GsrProcess>(stateProcess_)->flushCache();
    LazyObject::update();
}

void Gsr::updateTimes() const {
    volsteptimes_.clear();
    int j = 0;
    for (auto i = volstepdates_.begin(); i != volstepdates_.end(); ++i, ++j) {
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
    if (stateProcess_ != nullptr) {
        ext::static_pointer_cast<GsrProcess>(stateProcess_)->flushCache();
        ext::static_pointer_cast<GsrProcess>(stateProcess_)->setTimes(volsteptimesArray_);
    }
}

void Gsr::updateVolatility() {
    for (Size i = 0; i < sigma_.size(); i++) {
        sigma_.setParam(i, volatilities_[i]->value());
    }
    ext::static_pointer_cast<GsrProcess>(stateProcess_)->setVols(sigma_.params());
    update();
}

void Gsr::updateReversion() {
    for (Size i = 0; i < reversion_.size(); i++) {
        reversion_.setParam(i, reversions_[i]->value());
    }
    ext::static_pointer_cast<GsrProcess>(stateProcess_)->setReversions(reversion_.params());
    update();
}

void Gsr::initialize(Real T) {

    volsteptimesArray_ = Array(volstepdates_.size());

    updateTimes();

    QL_REQUIRE(volatilities_.size() == volsteptimes_.size() + 1,
               "there must be n+1 volatilities ("
                   << volatilities_.size() << ") for n volatility step times ("
                   << volsteptimes_.size() << ")");

    QL_REQUIRE(reversions_.size() == 1 ||
                   reversions_.size() == volsteptimes_.size() + 1,
               "there must be 1 or n+1 reversions ("
                   << reversions_.size() << ") for n volatility step times ("
                   << volsteptimes_.size() << ")");
    if (reversions_.size() == 1) {
        reversion_ = ConstantParameter(reversions_[0]->value(), NoConstraint());
    } else {
        reversion_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
        for (Size i = 0; i < reversion_.size(); i++) {
            reversion_.setParam(i, reversions_[i]->value());
        }
    }

    // sigma_ =
    // PiecewiseConstantParameter(volsteptimes_,PositiveConstraint());
    sigma_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
    for (Size i = 0; i < sigma_.size(); i++) {
        sigma_.setParam(i, volatilities_[i]->value());
    }

    stateProcess_ = ext::make_shared<GsrProcess>(
        volsteptimesArray_, sigma_.params(), reversion_.params(), T);

    registerWith(termStructure());

    registerWith(stateProcess_);

    volatilityObserver_ = ext::make_shared<VolatilityObserver>(this);
    reversionObserver_ = ext::make_shared<ReversionObserver>(this);

    for (auto& reversion : reversions_)
        reversionObserver_->registerWith(reversion);

    for (auto& volatilitie : volatilities_)
        volatilityObserver_->registerWith(volatilitie);
}

Real Gsr::zerobondImpl(const Time T, const Time t, const Real y,
                       const Handle<YieldTermStructure> &yts) const {

    calculate();

    if (t == 0.0)
        return yts.empty() ? this->termStructure()->discount(T, true)
                           : yts->discount(T, true);

    ext::shared_ptr<GsrProcess> p = ext::static_pointer_cast<GsrProcess>(stateProcess_);

    Real x = y * stateProcess_->stdDeviation(0.0, 0.0, t) +
             stateProcess_->expectation(0.0, 0.0, t);
    Real gtT = p->G(t, T, x);

    Real d = yts.empty()
                 ? termStructure()->discount(T, true) /
                       termStructure()->discount(t, true)
                 : yts->discount(T, true) / yts->discount(t, true);

    return d * exp(-x * gtT - 0.5 * p->y(t) * gtT * gtT);
}

Real Gsr::numeraireImpl(const Time t, const Real y,
                        const Handle<YieldTermStructure> &yts) const {

    calculate();

    ext::shared_ptr<GsrProcess> p = ext::static_pointer_cast<GsrProcess>(stateProcess_);

    if (t == 0)
        return yts.empty()
                   ? this->termStructure()->discount(p->getForwardMeasureTime(),
                                                     true)
                   : yts->discount(p->getForwardMeasureTime());
    return zerobond(p->getForwardMeasureTime(), t, y, yts);
}
}

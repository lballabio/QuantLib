/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/experimental/models/gsr.hpp>
#include <ql/quotes/simplequote.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {

Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
         const std::vector<Date> &volstepdates,
         const std::vector<Real> &volatilities, const Real reversion,
         const Real T, const std::vector<Real> &adjusters)
    : Gaussian1dModel(termStructure), CalibratedModel(3),
      reversion_(arguments_[0]), sigma_(arguments_[1]),
      adjuster_(arguments_[2]), volstepdates_(volstepdates) {

    QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");

    volatilities_.resize(volatilities.size());
    for (Size i = 0; i < volatilities.size(); ++i)
        volatilities_[i] =
            Handle<Quote>(boost::make_shared<SimpleQuote>(volatilities[i]));
    reversions_.resize(1);
    reversions_[0] = Handle<Quote>(boost::make_shared<SimpleQuote>(reversion));
    adjusters_.resize(adjusters.size() > 0 ? adjusters.size()
                                           : volatilities.size());
    for (Size i = 0; i < adjusters_.size(); ++i)
        adjusters_[i] = Handle<Quote>(boost::make_shared<SimpleQuote>(
            adjusters.size() > i ? adjusters[i] : 1.0));

    initialize(T);
}

Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
         const std::vector<Date> &volstepdates,
         const std::vector<Real> &volatilities,
         const std::vector<Real> &reversions, const Real T,
         const std::vector<Real> &adjusters)
    : Gaussian1dModel(termStructure), CalibratedModel(3),
      reversion_(arguments_[0]), sigma_(arguments_[1]),
      adjuster_(arguments_[2]), volstepdates_(volstepdates) {

    QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");

    volatilities_.resize(volatilities.size());
    for (Size i = 0; i < volatilities.size(); ++i)
        volatilities_[i] =
            Handle<Quote>(boost::make_shared<SimpleQuote>(volatilities[i]));
    reversions_.resize(adjusters.size() > 0 ? adjusters.size()
                                            : reversions.size());
    for (Size i = 0; i < reversions.size(); ++i)
        reversions_[i] =
            Handle<Quote>(boost::make_shared<SimpleQuote>(reversions[i]));
    adjusters_.resize(volatilities.size());
    for (Size i = 0; i < adjusters_.size(); ++i)
        adjusters_[i] = Handle<Quote>(boost::make_shared<SimpleQuote>(
            adjusters.size() > i ? adjusters[i] : 1.0));

    initialize(T);
}

Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
         const std::vector<Date> &volstepdates,
         const std::vector<Handle<Quote> > &volatilities,
         const Handle<Quote> reversion, const Real T,
         const std::vector<Handle<Quote> > &adjusters)
    : Gaussian1dModel(termStructure), CalibratedModel(3),
      reversion_(arguments_[0]), sigma_(arguments_[1]),
      adjuster_(arguments_[2]), volatilities_(volatilities),
      reversions_(std::vector<Handle<Quote> >(1, reversion)),
      adjusters_(
          adjusters.size() > 0
              ? adjusters
              : std::vector<Handle<Quote> >(
                    volatilities.size(),
                    Handle<Quote>(boost::make_shared<SimpleQuote>(1.0)))),
      volstepdates_(volstepdates) {

    QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");
    initialize(T);
}

Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
         const std::vector<Date> &volstepdates,
         const std::vector<Handle<Quote> > &volatilities,
         const std::vector<Handle<Quote> > &reversions, const Real T,
         const std::vector<Handle<Quote> > &adjusters)
    : Gaussian1dModel(termStructure), CalibratedModel(3),
      reversion_(arguments_[0]), sigma_(arguments_[1]),
      adjuster_(arguments_[2]), volatilities_(volatilities),
      reversions_(reversions),
      adjusters_(
          adjusters.size() > 0
              ? adjusters
              : std::vector<Handle<Quote> >(
                    volatilities.size(),
                    Handle<Quote>(boost::make_shared<SimpleQuote>(1.0)))),
      volstepdates_(volstepdates) {

    QL_REQUIRE(!termStructure.empty(), "yield term structure handle is empty");
    initialize(T);
}

void Gsr::updateTimes() const {
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
    if (stateProcess_ != NULL)
        boost::static_pointer_cast<GsrProcess>(stateProcess_)->flushCache();
}

void Gsr::updateState() const {
    for (Size i = 0; i < sigma_.size(); i++) {
        sigma_.setParam(i, volatilities_[i]->value());
    }
    for (Size i = 0; i < reversion_.size(); i++) {
        reversion_.setParam(i, reversions_[i]->value());
    }
    boost::static_pointer_cast<GsrProcess>(stateProcess_)->flushCache();
}

void Gsr::initialize(Real T) {

    volsteptimesArray_ = Array(volstepdates_.size());

    updateTimes();

    QL_REQUIRE(volatilities_.size() == volsteptimes_.size() + 1,
               "there must be n+1 volatilities ("
                   << volatilities_.size() << ") for n volatility step times ("
                   << volsteptimes_.size() << ")");
    QL_REQUIRE(adjusters_.size() == volatilities_.size(),
               "there must be as many adjusters ("
                   << adjusters_.size() << ") as volatilities ("
                   << volatilities_.size() << ")");
    // sigma_ =
    // PiecewiseConstantParameter(volsteptimes_,PositiveConstraint());
    sigma_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
    adjuster_ = PiecewiseConstantParameter(volsteptimes_, PositiveConstraint());
    unitAdjuster_ = PiecewiseConstantParameter(volsteptimes_, PositiveConstraint());

    QL_REQUIRE(reversions_.size() == 1 ||
                   reversions_.size() == volsteptimes_.size() + 1,
               "there must be 1 or n+1 reversions ("
                   << reversions_.size() << ") for n volatility step times ("
                   << volsteptimes_.size() << ")");
    if (reversions_.size() == 1) {
        reversion_ = ConstantParameter(reversions_[0]->value(), NoConstraint());
    } else {
        reversion_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
    }

    for (Size i = 0; i < sigma_.size(); i++) {
        sigma_.setParam(i, volatilities_[i]->value());
    }
    for (Size i = 0; i < adjuster_.size(); i++) {
        adjuster_.setParam(i, adjusters_[i]->value());
        unitAdjuster_.setParam(i,1.0);
    }
    for (Size i = 0; i < reversion_.size(); i++) {
        reversion_.setParam(i, reversions_[i]->value());
    }

    stateProcess_ = boost::shared_ptr<GsrProcess>(
        new GsrProcess(volsteptimesArray_, sigma_.params(), reversion_.params(),
                       unitAdjuster_.params(), T));

    adjustedStateProcess_ = boost::shared_ptr<GsrProcess>(
        new GsrProcess(volsteptimesArray_, sigma_.params(), reversion_.params(),
                       adjuster_.params(), T));

    registerWith(termStructure());

    registerWith(stateProcess_);
    for (Size i = 0; i < reversions_.size(); ++i)
        registerWith(reversions_[i]);

    for (Size i = 0; i < volatilities_.size(); ++i)
        registerWith(volatilities_[i]);

    for (Size i = 0; i < adjusters_.size(); ++i)
        registerWith(adjusters_[i]);
}

const Real Gsr::zerobondImpl(const Time T, const Time t, const Real y,
                             const Handle<YieldTermStructure> &yts,
                             const bool adjusted) const {

    calculate();

    if (t == 0.0)
        return yts.empty() ? this->termStructure()->discount(T, true)
                           : yts->discount(T, true);

    boost::shared_ptr<GsrProcess> p = adjusted ?
        boost::dynamic_pointer_cast<GsrProcess>(adjustedStateProcess_) :
        boost::dynamic_pointer_cast<GsrProcess>(stateProcess_);

    Real x = y * stateProcess_->stdDeviation(0.0, 0.0, t) +
             stateProcess_->expectation(0.0, 0.0, t);
    Real gtT = p->G(t, T, x);

    Real d = yts.empty()
                 ? termStructure()->discount(T, true) /
                       termStructure()->discount(t, true)
                 : yts->discount(T, true) / yts->discount(t, true);

    return d * exp(-x * gtT - 0.5 * p->y(t) * gtT * gtT);
}

const Real Gsr::numeraireImpl(const Time t, const Real y,
                              const Handle<YieldTermStructure> &yts) const {

    calculate();

    boost::shared_ptr<GsrProcess> p =
        boost::dynamic_pointer_cast<GsrProcess>(stateProcess_);

    if (t == 0)
        return yts.empty()
                   ? this->termStructure()->discount(p->getForwardMeasureTime(),
                                                     true)
                   : yts->discount(p->getForwardMeasureTime());
    return zerobond(p->getForwardMeasureTime(), t, y, yts, false);
}
}

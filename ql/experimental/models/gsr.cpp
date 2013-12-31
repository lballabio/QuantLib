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

#include <ql/experimental/models/gsr.hpp>

namespace QuantLib {

    Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
             const std::vector<Date> &volstepdates,
             const std::vector<Real> &volatilities, const Real reversion,
             const Real T)
        : Gaussian1dModel(termStructure), CalibratedModel(2),
          reversion_(arguments_[0]), sigma_(arguments_[1]),
          volatilities_(volatilities),
          reversions_(std::vector<Real>(1, reversion)),
          volstepdates_(volstepdates) {

        QL_REQUIRE(!termStructure.empty(),
                   "yield term structure handle is empty");
        initialize(T);
    }

    Gsr::Gsr(const Handle<YieldTermStructure> &termStructure,
             const std::vector<Date> &volstepdates,
             const std::vector<Real> &volatilities,
             const std::vector<Real> &reversions, const Real T)
        : Gaussian1dModel(termStructure), CalibratedModel(2),
          reversion_(arguments_[0]), sigma_(arguments_[1]),
          volatilities_(volatilities), reversions_(reversions),
          volstepdates_(volstepdates) {

        QL_REQUIRE(!termStructure.empty(),
                   "yield term structure handle is empty");
        initialize(T);
    }

    void Gsr::initialize(Real T) {

        volsteptimes_.clear();
        volsteptimesArray_ = Array(volstepdates_.size());
        int j = 0;
        for (std::vector<Date>::const_iterator i = volstepdates_.begin();
             i != volstepdates_.end(); i++, j++) {
            volsteptimes_.push_back(termStructure()->timeFromReference(*i));
            volsteptimesArray_[j] = volsteptimes_[j];
            if (j == 0)
                QL_REQUIRE(volsteptimes_[0] > 0.0,
                           "volsteptimes must be positive (" << volsteptimes_[0]
                                                             << ")");
            else
                QL_REQUIRE(volsteptimes_[j] > volsteptimes_[j - 1],
                           "volsteptimes must be strictly increasing ("
                               << volsteptimes_[j - 1] << "@" << (j - 1) << ", "
                               << volsteptimes_[j] << "@" << j << ")");
        }

        QL_REQUIRE(volatilities_.size() == volsteptimes_.size() + 1,
                   "there must be n+1 volatilities ("
                       << volatilities_.size()
                       << ") for n volatility step times ("
                       << volsteptimes_.size() << ")");
        // sigma_ =
        // PiecewiseConstantParameter(volsteptimes_,PositiveConstraint());
        sigma_ = PiecewiseConstantParameter(volsteptimes_, NoConstraint());
        for (Size i = 0; i < sigma_.size(); i++) {
            sigma_.setParam(i, volatilities_[i]);
        }

        QL_REQUIRE(reversions_.size() == 1 ||
                       reversions_.size() == volsteptimes_.size() + 1,
                   "there must be 1 or n+1 reversions ("
                       << reversions_.size()
                       << ") for n volatility step times ("
                       << volsteptimes_.size() << ")");
        if (reversions_.size() == 1) {
            reversion_ = ConstantParameter(reversions_[0], NoConstraint());
        } else {
            reversion_ =
                PiecewiseConstantParameter(volsteptimes_, NoConstraint());
            for (Size i = 0; i < reversions_.size(); i++) {
                reversion_.setParam(i, reversions_[i]);
            }
        }

        stateProcess_ = boost::shared_ptr<GsrProcess>(new GsrProcess(
            volsteptimesArray_, sigma_.params(), reversion_.params(), T));

        LazyObject::registerWith(stateProcess_); // forward measure time may
                                                 // change, the model must be
                                                 // notified then
        LazyObject::registerWith(termStructure());
    }

    const Real Gsr::zerobondImpl(const Time T, const Time t, const Real y,
                                 const Handle<YieldTermStructure> &yts) const {

        if (t == 0.0)
            return yts.empty() ? this->termStructure()->discount(T, true)
                               : yts->discount(T, true);

        calculate();

        boost::shared_ptr<GsrProcess> p =
            boost::dynamic_pointer_cast<GsrProcess>(stateProcess_);

        Real x = y * p->stdDeviation(0.0, 0.0, t) +
                 stateProcess_->expectation(0.0, 0.0, t);
        Real gtT = p->G(t, T, x);

        Real d = yts.empty() ? termStructure()->discount(T, true) /
                                   termStructure()->discount(t, true)
                             : yts->discount(T, true) / yts->discount(t, true);

        return d * exp(-x * gtT - 0.5 * p->y(t) * gtT * gtT);
    }

    const Real Gsr::numeraireImpl(const Time t, const Real y,
                                  const Handle<YieldTermStructure> &yts) const {

        boost::shared_ptr<GsrProcess> p =
            boost::dynamic_pointer_cast<GsrProcess>(stateProcess_);

        if (t == 0)
            return yts.empty() ? this->termStructure()->discount(
                                     p->getForwardMeasureTime(), true)
                               : yts->discount(p->getForwardMeasureTime());
        return zerobond(p->getForwardMeasureTime(), t, y, yts);
    }

}

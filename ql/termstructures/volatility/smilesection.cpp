/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
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

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    void SmileSection::update() {
        if (isFloating_) {
            referenceDate_ = Settings::instance().evaluationDate();
            initializeExerciseTime();
        }
    }

    void SmileSection::initializeExerciseTime() const {
        QL_REQUIRE(exerciseDate_>=referenceDate_,
                   "expiry date (" << exerciseDate_ <<
                   ") must be greater than reference date (" <<
                   referenceDate_ << ")");
        exerciseTime_ = dc_.yearFraction(referenceDate_, exerciseDate_);
    }

    SmileSection::SmileSection(const Date& d,
                               const DayCounter& dc,
                               const Date& referenceDate)
    : exerciseDate_(d), dc_(dc) {
        isFloating_ = referenceDate==Date();
        if (isFloating_) {
            registerWith(Settings::instance().evaluationDate());
            referenceDate_ = Settings::instance().evaluationDate();
        } else
            referenceDate_ = referenceDate;
        initializeExerciseTime();
    }

    SmileSection::SmileSection(Time exerciseTime,
                               const DayCounter& dc)
    : isFloating_(false), referenceDate_(Date()),
      dc_(dc), exerciseTime_(exerciseTime) {
        QL_REQUIRE(exerciseTime_>=0.0,
                   "expiry time must be positive: " <<
                   exerciseTime_ << " not allowed");
    }

    Real SmileSection::optionPrice(Rate strike,
                                   Option::Type type,
                                   Real discount) const {
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute option price");
        // for zero strike, return option price even if outside
        // minstrike, maxstrike interval
        return blackFormula(type,strike,atm, fabs(strike) < QL_EPSILON ?
                            0.2 : sqrt(variance(strike)),discount);
    }

    Real SmileSection::digitalOptionPrice(Rate strike,
                                          Option::Type type,
                                          Real discount,
                                          Real gap) const {
        Real kl = std::max(strike-gap/2.0,0.0);
        Real kr = kl+gap;
        return (type==Option::Call ? 1.0 : -1.0) *
            (optionPrice(kl,type,discount)-optionPrice(kr,type,discount)) / gap;
    }
    
    Real SmileSection::density(Rate strike, Real discount, Real gap) const {
        Real kl = std::max(strike-gap/2.0,0.0);
        Real kr = kl+gap;
        return (digitalOptionPrice(kl,Option::Call,discount,gap) -
                digitalOptionPrice(kr,Option::Call,discount,gap)) / gap;
    }

    Real SmileSection::vega(Rate strike, Real discount) const {
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute option price");
        return blackFormulaVolDerivative(strike,atmLevel(),
                                         sqrt(variance(strike)),
                                         exerciseTime(),discount)*0.01;
    }

}

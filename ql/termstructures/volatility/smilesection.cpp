/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2013, 2015 Peter Caspers
 Copyright (C) 2026 Yassine Idyiahia

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

#include <ql/math/comparison.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

using std::sqrt;

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
                               DayCounter dc,
                               const Date& referenceDate,
                               const VolatilityType type,
                               const Rate shift)
    : exerciseDate_(d), dc_(std::move(dc)), volatilityType_(type), shift_(shift) {
        isFloating_ = referenceDate==Date();
        if (isFloating_) {
            registerWith(Settings::instance().evaluationDate());
            referenceDate_ = Settings::instance().evaluationDate();
        } else
            referenceDate_ = referenceDate;
        SmileSection::initializeExerciseTime();
    }

    SmileSection::SmileSection(Time exerciseTime,
                               DayCounter dc,
                               const VolatilityType type,
                               const Rate shift)
    : isFloating_(false), dc_(std::move(dc)), exerciseTime_(exerciseTime), volatilityType_(type),
      shift_(shift) {
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
        // if lognormal or shifted lognormal,
        // for strike at -shift, return option price even if outside
        // minstrike, maxstrike interval
        if (volatilityType() == ShiftedLognormal)
            return blackFormula(type,strike,atm, std::fabs(strike+shift()) < QL_EPSILON ?
                            0.2 : Real(sqrt(variance(strike))),discount,shift());
        else
            return bachelierBlackFormula(type,strike,atm,sqrt(variance(strike)),discount);
    }

    Real SmileSection::digitalOptionPrice(Rate strike,
                                          Option::Type type,
                                          Real discount,
                                          Real gap) const {
        Real m = volatilityType() == ShiftedLognormal ? Real(-shift()) : -QL_MAX_REAL;
        // A fixed gap loses meaning once it drops below the strike's own
        // precision: the interval quantizes, and collapses entirely when kl and
        // kr coincide. The floor sits far below any usual gap.
        Real g = std::max(gap, std::abs(strike)*1.0e-10);
        Real kl = std::max(strike-g/2.0,m);
        Real kr = kl+g;
        return (type==Option::Call ? 1.0 : -1.0) *
            (optionPrice(kl,type,discount)-optionPrice(kr,type,discount)) / g;
    }

    Real SmileSection::density(Rate strike, Real discount, Real gap) const {
        Real m = volatilityType() == ShiftedLognormal ? Real(-shift()) : -QL_MAX_REAL;
        // A second difference, so roundoff scales as |strike|*eps/g^2 rather
        // than the digital's |strike|*eps/g. The floor has to be
        // correspondingly larger. The same g must reach the digitals and the
        // divisor, or their own floor leaves the three inconsistent.
        Real g = std::max(gap, std::abs(strike)*1.0e-6);
        Real kl = std::max(strike-g/2.0,m);
        Real kr = kl+g;
        return (digitalOptionPrice(kl,Option::Call,discount,g) -
                digitalOptionPrice(kr,Option::Call,discount,g)) / g;
    }

    Real SmileSection::vega(Rate strike, Real discount) const {
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute option vega");
        if (volatilityType() == ShiftedLognormal)
            return blackFormulaVolDerivative(strike,atmLevel(),
                                             sqrt(variance(strike)),
                                             exerciseTime(),discount,shift())*0.01;
        else
            QL_FAIL("vega for normal smilesection not yet implemented");
    }

    Real SmileSection::volatility(Rate strike, VolatilityType volatilityType,
                                  Real shift) const {
        if(volatilityType == volatilityType_ && close(shift,this->shift()))
            return volatility(strike);
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute converted volatilties");
        Option::Type type = strike >= atm ? Option::Call : Option::Put;
        Real premium = optionPrice(strike,type);
        Real premiumAtm = optionPrice(atm,type);
        if (volatilityType == ShiftedLognormal) {
            try {
                return blackFormulaImpliedStdDev(type, strike, atm, premium,
                                                 1.0, shift) /
                       std::sqrt(exerciseTime());
            } catch(...) {
                return blackFormulaImpliedStdDevChambers(
                    type, strike, atm, premium, premiumAtm, 1.0, shift) /
                       std::sqrt(exerciseTime());
            }
        } else {
                return bachelierBlackFormulaImpliedVol(type, strike, atm,
                                                       exerciseTime(), premium);
            }
    }
}

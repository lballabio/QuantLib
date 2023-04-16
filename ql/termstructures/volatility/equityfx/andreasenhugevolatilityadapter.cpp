/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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

#include <ql/math/functional.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityadapter.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityinterpl.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    AndreasenHugeVolatilityAdapter::AndreasenHugeVolatilityAdapter(
        std::shared_ptr<AndreasenHugeVolatilityInterpl> volInterpl, Real eps)
    : eps_(eps), volInterpl_(std::move(volInterpl)) {}

    Real AndreasenHugeVolatilityAdapter::blackVarianceImpl(Time t, Real strike)
    const {
        const Real fwd = volInterpl_->fwd(t);
        const Option::Type optionType =
            (fwd > strike)? Option::Put : Option::Call;

        const Real npv = volInterpl_->optionPrice(t, strike, optionType);

        return squared(blackFormulaImpliedStdDevLiRS(
            optionType, strike, fwd, npv,
            volInterpl_->riskFreeRate()->discount(t),
            0.0, Null<Real>(), 1.0, eps_, 1000));
    }


    Date AndreasenHugeVolatilityAdapter::maxDate() const {
        return volInterpl_->maxDate();
    }
    Real AndreasenHugeVolatilityAdapter::minStrike() const {
        return volInterpl_->minStrike();
    }
    Real AndreasenHugeVolatilityAdapter::maxStrike() const {
        return volInterpl_->maxStrike();
    }
    Calendar AndreasenHugeVolatilityAdapter::calendar() const {
        return volInterpl_->riskFreeRate()->calendar();
    }
    DayCounter AndreasenHugeVolatilityAdapter::dayCounter() const {
        return volInterpl_->riskFreeRate()->dayCounter();
    }
    const Date& AndreasenHugeVolatilityAdapter::referenceDate() const {
        return volInterpl_->riskFreeRate()->referenceDate();
    }
    Natural AndreasenHugeVolatilityAdapter::settlementDays() const {
        return volInterpl_->riskFreeRate()->settlementDays();
    }
}

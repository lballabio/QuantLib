/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017, 2018 Klaus Spanderen

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

#include <ql/termstructures/volatility/equityfx/andreasenhugelocalvoladapter.hpp>
#include <ql/termstructures/volatility/equityfx/andreasenhugevolatilityinterpl.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {


    AndreasenHugeLocalVolAdapter::AndreasenHugeLocalVolAdapter(
        std::shared_ptr<AndreasenHugeVolatilityInterpl> localVol)
    : localVol_(std::move(localVol)) {}

    Date AndreasenHugeLocalVolAdapter::maxDate() const {
        return localVol_->maxDate();
    }

    Real AndreasenHugeLocalVolAdapter::minStrike() const {
        return 0.0;
    }

    Real AndreasenHugeLocalVolAdapter::maxStrike() const {
        return QL_MAX_REAL;
    }

    Volatility
    AndreasenHugeLocalVolAdapter::localVolImpl(Time t, Real strike) const {
        return localVol_->localVol(t,
            std::min(localVol_->maxStrike(),
                std::max(localVol_->minStrike(), strike)));
    }

    Calendar AndreasenHugeLocalVolAdapter::calendar() const {
        return localVol_->riskFreeRate()->calendar();
    }
    DayCounter AndreasenHugeLocalVolAdapter::dayCounter() const {
        return localVol_->riskFreeRate()->dayCounter();
    }
    const Date& AndreasenHugeLocalVolAdapter::referenceDate() const {
        return localVol_->riskFreeRate()->referenceDate();
    }
    Natural AndreasenHugeLocalVolAdapter::settlementDays() const {
        return localVol_->riskFreeRate()->settlementDays();
    }
}

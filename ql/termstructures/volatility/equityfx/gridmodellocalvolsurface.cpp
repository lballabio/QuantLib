/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file parameterizedlocalvolsurface.cpp
*/

#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/gridmodellocalvolsurface.hpp>
#include <ql/functional.hpp>
#include <algorithm>

namespace QuantLib {
    GridModelLocalVolSurface::GridModelLocalVolSurface(
        const Date& referenceDate,
        const std::vector<Date>& dates,
        const std::vector<ext::shared_ptr<std::vector<Real> > >& strikes,
        const DayCounter& dayCounter,
        Extrapolation lowerExtrapolation,
        Extrapolation upperExtrapolation)
    : LocalVolTermStructure(
            referenceDate, NullCalendar(), Following, dayCounter),
      CalibratedModel(dates.size()*strikes.front()->size()),
      referenceDate_(referenceDate),
      times_(dates.size()),
      strikes_(strikes),
      dayCounter_(dayCounter),
      lowerExtrapolation_(lowerExtrapolation),
      upperExtrapolation_(upperExtrapolation) {

        for (Size i=1; i < strikes_.size(); ++i) {
            QL_REQUIRE(strikes_[i]->size() == strikes_.front()->size(),
                       "strike vectors must have the same dimension");
        }

        std::fill(arguments_.begin(), arguments_.end(),
            ConstantParameter(1.0, PositiveConstraint()));

        for (Size i=0; i < dates.size(); ++i) {
            times_[i] = dayCounter.yearFraction(referenceDate_, dates[i]);
        }

        generateArguments();
    }

    void GridModelLocalVolSurface::update() {
        LocalVolTermStructure::update();
        CalibratedModel::update();
    }

    Date GridModelLocalVolSurface::maxDate() const {
        return localVol_->maxDate();
    }
    Time GridModelLocalVolSurface::maxTime() const {
        return localVol_->maxTime();
    }
    Real GridModelLocalVolSurface::minStrike() const {
        return localVol_->minStrike();
    }
    Real GridModelLocalVolSurface::maxStrike() const {
        return localVol_->maxStrike();
    }

    Volatility GridModelLocalVolSurface::localVolImpl(Time t, Real strike)
    const {
        return localVol_->localVol(t, strike, true);
    }

    void GridModelLocalVolSurface::generateArguments() {
        using namespace ext::placeholders;
        const ext::shared_ptr<Matrix> localVolMatrix(
            new Matrix(strikes_.front()->size(), times_.size()));

        std::transform(arguments_.begin(), arguments_.end(),
                       localVolMatrix->begin(),
                       [](const Parameter& p) { return p(0.0); });

        localVol_ = ext::make_shared<FixedLocalVolSurface>(
                referenceDate_,
                times_,
                strikes_,
                localVolMatrix,
                dayCounter_,
                lowerExtrapolation_,
                upperExtrapolation_);
    }
}

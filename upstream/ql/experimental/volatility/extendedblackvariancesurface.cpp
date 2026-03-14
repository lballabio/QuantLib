/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Frank Hövermann

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

#include <ql/experimental/volatility/extendedblackvariancesurface.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <utility>

namespace QuantLib {

    ExtendedBlackVarianceSurface::ExtendedBlackVarianceSurface(
        const Date& referenceDate,
        const Calendar& calendar,
        const std::vector<Date>& dates,
        std::vector<Real> strikes,
        const std::vector<Handle<Quote> >& volatilities,
        DayCounter dayCounter,
        ExtendedBlackVarianceSurface::Extrapolation lowerEx,
        ExtendedBlackVarianceSurface::Extrapolation upperEx)
    : BlackVarianceTermStructure(referenceDate, calendar), dayCounter_(std::move(dayCounter)),
      maxDate_(dates.back()), volatilities_(volatilities), strikes_(std::move(strikes)),
      lowerExtrapolation_(lowerEx), upperExtrapolation_(upperEx) {

        QL_REQUIRE(dates.size()*strikes_.size()==volatilities_.size(),
                   "size mismatch between date vector and vol matrix columns "
                   "and/or between money-strike vector and vol matrix rows");

        QL_REQUIRE(dates[0] > referenceDate,
                   "cannot have dates_[0] <= referenceDate_");


        times_ = std::vector<Time>(dates.size()+1);
        times_[0] = 0.0;

        for (Size j=1; j<=dates.size(); j++) {
            times_[j] = timeFromReference(dates[j-1]);
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique");
        }

        variances_ = Matrix(strikes_.size(), dates.size()+1);
        setVariances();

        setInterpolation<Bilinear>();

        for (const auto& volatilitie : volatilities_)
            registerWith(volatilitie);
    }

    void ExtendedBlackVarianceSurface::setVariances() {

        for (Size i=0; i<times_.size()+1; i++) {
            variances_[0][i] = 0.0;
        }
        for (Size j=1; j<=times_.size(); j++) {
            for (Size i=0; i<strikes_.size(); i++) {
                Volatility sigma = volatilities_[i*times_.size()+j-1]->value();
                variances_[i][j] = times_[j] * sigma * sigma;
                QL_REQUIRE(variances_[i][j]>=variances_[i][j-1],
                           "variance must be non-decreasing");
            }
        }
    }

    void ExtendedBlackVarianceSurface::update() {
        setVariances();
        varianceSurface_.update();
        notifyObservers();
    }

    Real ExtendedBlackVarianceSurface::blackVarianceImpl(Time t,
                                                         Real strike) const {

        if (t==0.0) return 0.0;

        // enforce constant extrapolation when required
        if (strike < strikes_.front()
            && lowerExtrapolation_ == ConstantExtrapolation)
            strike = strikes_.front();
        if (strike > strikes_.back()
            && upperExtrapolation_ == ConstantExtrapolation)
            strike = strikes_.back();

        if (t<=times_.back())
            return varianceSurface_(t, strike, true);
        else // t>times_.back() || extrapolate
            return varianceSurface_(times_.back(), strike, true) *
                t/times_.back();
    }

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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

#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <utility>

namespace QuantLib {

    BlackVarianceSurface::BlackVarianceSurface(const Date& referenceDate,
                                               const Calendar& cal,
                                               const std::vector<Date>& dates,
                                               std::vector<Real> strikes,
                                               const Matrix& blackVolMatrix,
                                               DayCounter dayCounter,
                                               BlackVarianceSurface::Extrapolation lowerEx,
                                               BlackVarianceSurface::Extrapolation upperEx)
    : BlackVarianceTermStructure(referenceDate, cal), dayCounter_(std::move(dayCounter)),
      maxDate_(dates.back()), strikes_(std::move(strikes)), lowerExtrapolation_(lowerEx),
      upperExtrapolation_(upperEx) {

        QL_REQUIRE(dates.size()==blackVolMatrix.columns(),
                   "mismatch between date vector and vol matrix colums");
        QL_REQUIRE(strikes_.size()==blackVolMatrix.rows(),
                   "mismatch between money-strike vector and vol matrix rows");

        QL_REQUIRE(dates[0]>=referenceDate,
                   "cannot have dates[0] < referenceDate");

        Size j, i;
        times_ = std::vector<Time>(dates.size()+1);
        times_[0] = 0.0;
        variances_ = Matrix(strikes_.size(), dates.size()+1);
        for (i=0; i<blackVolMatrix.rows(); i++) {
            variances_[i][0] = 0.0;
        }
        for (j=1; j<=blackVolMatrix.columns(); j++) {
            times_[j] = timeFromReference(dates[j-1]);
            QL_REQUIRE(times_[j]>times_[j-1],
                       "dates must be sorted unique!");
            for (i=0; i<blackVolMatrix.rows(); i++) {
                variances_[i][j] = times_[j] *
                    blackVolMatrix[i][j-1]*blackVolMatrix[i][j-1];
            }
        }
        // default: bilinear interpolation
        setInterpolation<Bilinear>();
    }

    Real BlackVarianceSurface::blackVarianceImpl(Time t, Real strike) const {

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


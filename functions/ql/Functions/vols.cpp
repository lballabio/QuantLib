/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Functions/vols.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Volatilities/localconstantvol.hpp>
#include <ql/Volatilities/localvolcurve.hpp>
#include <ql/Volatilities/blackvariancesurface.hpp>


namespace QuantLib {

    Volatility blackVol(const Date& refDate,
                        const DayCounter& dayCounter,
                        const std::vector<Date>& dates,
                        const std::vector<Real>& strikes,
                        const Matrix& blackVolSurface,
                        const Date& date1,
                        const Date& date2,
                        Real strike,
                        Integer interpolation2DType,
                        bool allowExtrapolation) {

        typedef BlackVarianceSurface surface_t;

        Real result = 0.0;

        switch (interpolation2DType) {
          case 1:
            result = surface_t(
                refDate, dates, strikes, blackVolSurface, dayCounter,
                surface_t::InterpolatorDefaultExtrapolation,
                surface_t::InterpolatorDefaultExtrapolation).blackForwardVol(
                    date1, date2, strike, allowExtrapolation);
            break;
          default:
            QL_FAIL("invalid interpolation type");
        }

        return result;
    }

}

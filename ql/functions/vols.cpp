/*
 Copyright (C) 2002 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file vols.cpp
    \brief Volatility functions

    \fullpath
    ql/functions/%vols.cpp
*/

// $Id$

#include <ql/functions/vols.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Volatilities/constantvol.hpp>
#include <ql/Volatilities/blackvariancesurface.hpp>

using QuantLib::Date;
using QuantLib::Math::Matrix;
using QuantLib::Math::Interpolation2D;
using QuantLib::Math::BilinearInterpolation;
using QuantLib::VolTermStructures::BlackVarianceSurface;

namespace QuantLib {

    namespace Functions {

		double blackVol(const Date& refDate,
                        const DayCounter& dc,
                        const std::vector<Date>& dates,
                        const std::vector<double>& strikes,
                        const QuantLib::Math::Matrix& blackVolSurface,
                        const Date& date1,
                        const Date& date2,
                        double strike,
                        int interpolation2DType,
                        bool allowExtrapolation) {

            double result = 0.0;

            switch (interpolation2DType) {
                case 1:
                    result = BlackVarianceSurface<
                        BilinearInterpolation<
                        std::vector<double>::const_iterator,
			            std::vector<double>::const_iterator,
                        Matrix> >(refDate, dc, dates, strikes,
                        blackVolSurface).blackForwardVol(date1, date2,
                        strike, allowExtrapolation);
                    break;
                default:
                    throw IllegalArgumentError(
                        "interpolate2D: invalid interpolation type");
            }

            return result;
        }

    }

}

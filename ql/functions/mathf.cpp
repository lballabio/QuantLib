
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
/*! \file mathf.cpp
    \brief math functions

    \fullpath
    ql/functions/%mathf.cpp
*/

// $Id$

#include <ql/functions/mathf.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/Math/bilinearinterpolation.hpp>

using QuantLib::Array;
using QuantLib::Math::Matrix;
using QuantLib::Math::Interpolation;
using QuantLib::Math::LinearInterpolation;
using QuantLib::Math::CubicSpline;
using QuantLib::Math::Interpolation2D;
using QuantLib::Math::BilinearInterpolation;

namespace QuantLib {

    namespace Functions {

		double interpolate(
		    const std::vector<double>& x_values,
            const std::vector<double>& y_values, double x,
            int interpolationType,
            bool allowExtrapolation) {

            double result = 0.0;
            
            
            switch (interpolationType) {
                case 1:
                    result = LinearInterpolation<
                        std::vector<double>::const_iterator, 
			            std::vector<double>::const_iterator>(
                        x_values.begin(), x_values.end(),
                        y_values.begin(), allowExtrapolation)(x);
                    break;
                case 2:
                    result = CubicSpline<
                        std::vector<double>::const_iterator, 
			            std::vector<double>::const_iterator>(
                        x_values.begin(), x_values.end(),
                        y_values.begin(), allowExtrapolation)(x);
                    break;
                default:
                    throw IllegalArgumentError(
                        "interpolate: invalid interpolation type");
            }
                
			return result;
        }

		double interpolate2D(const std::vector<double>& x_values,
            const std::vector<double>& y_values, const Matrix& dataMatrix,
            double x, double y, int interpolation2DType,
            bool allowExtrapolation) {

            double result = 0.0;

            switch (interpolation2DType) {
                case 1:
                    result = BilinearInterpolation<
                        std::vector<double>::const_iterator,
			            std::vector<double>::const_iterator,
                        Matrix>(x_values.begin(), x_values.end(),
                        y_values.begin(), y_values.end(), dataMatrix,
                        allowExtrapolation)(x,y);
                    break;
                default:
                    throw IllegalArgumentError(
                        "interpolate2D: invalid interpolation type");
            }

            return result;
        }

    }

}

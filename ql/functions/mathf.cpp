

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
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>

using QuantLib::Array;
using QuantLib::Math::Matrix;
using QuantLib::Math::BilinearInterpolation;
using QuantLib::Math::LinearInterpolation;

namespace QuantLib {

    namespace Functions {
		double interpolate2D(std::vector<double>& x_values,
            std::vector<double>& y_values, Matrix& dataMatrix,
            double x, double y) {
			return BilinearInterpolation<std::vector<double>::iterator, std::vector<double>::iterator>(
                x_values.begin(), y_values.begin(), dataMatrix)(x,y);
        }

		double interpolate(std::vector<double>& x_values,
            std::vector<double>& y_values, double x) {
			return LinearInterpolation<std::vector<double>::iterator, std::vector<double>::iterator>(
                x_values.begin(), x_values.end(), y_values.begin())(x);
        }


    }

}

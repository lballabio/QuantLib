
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
/*! \file mathf.hpp
    \brief math functions

    \fullpath
    ql/functions/%mathf.hpp
*/

// $Id$

#ifndef quantlib_functions_math_h
#define quantlib_functions_math_h

#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib {

    namespace Functions {

        double interpolate(
            const std::vector<double>& x_values,
            const std::vector<double>& y_values,
            double x,
            int interpolationType,
            bool allowExtrapolation);

        double interpolate2D(
            const std::vector<double>& x_values,
            const std::vector<double>& y_values,
            const Math::Matrix& dataMatrix,
            double x,
            double y,
            int interpolation2DType,
            bool allowExtrapolation);



        double normDist(
            double x,
            double mean = 0.0,
            double standard_dev = 1.0,
            bool cumulative = false);

        double normInv(double probability,
            double mean = 0.0,
            double standard_dev = 1.0);

    }

}


#endif

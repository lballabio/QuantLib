

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

#include <ql/Math/bilinearinterpolation.hpp>

namespace QuantLib {

    namespace Functions {
        double interpolate2D(Array& x_values, Array& y_values, Math::Matrix& dataMatrix,
            double x, double y);
    }

}


#endif

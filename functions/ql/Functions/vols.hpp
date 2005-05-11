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

#ifndef quantlib_functions_vols_h
#define quantlib_functions_vols_h

#include <ql/Functions/qlfunctions.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/daycounter.hpp>
#include <vector>

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
                        bool allowExtrapolation);

}


#endif

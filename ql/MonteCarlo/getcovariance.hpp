
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/

// $Id$

#ifndef quantlib_montecarlo_get_covariance_h
#define quantlib_montecarlo_get_covariance_h

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        /*! Combines the correlation matrix and the vector of volatilities
            to return the covariance matrix.
            Note that only the symmetric part of the correlation matrix is
            used. Also it is assumed that the diagonal member of the
            correlation matrix equals one.
        */
        Math::Matrix getCovariance(const Array& volatilities,
            const Math::Matrix& correlations);

    }

}


#endif

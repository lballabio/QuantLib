/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 *          http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file getcovariance.hpp
    \brief Returns a covarinace matrix given a correlation matrix and a volatility array

    $Source$
    $Log$
    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/15 13:48:38  marmar
    getCovariance function added

*/

#ifndef quantlib_montecarlo_get_covariance_h
#define quantlib_montecarlo_get_covariance_h

#include "qldefines.hpp"
#include "qlerrors.hpp"
#include "array.hpp"
#include "Math/matrix.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        /*! Combines the correlation matrix and the vector of volatilities
            to return the covariance matrix.
            Note that only the symmetric part of the correlation matrix is
            used. Also it is assumed that the diagonal member of the 
            correlation matrix equals one.
        */
        Math::Matrix getCovariance(const Array &volatilities,
                                   const Math::Matrix &correlations);

    }

}

#endif

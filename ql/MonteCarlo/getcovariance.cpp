
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file getcovariance.cpp
    \brief Covariance matrix calculation

    \fullpath
    ql/MonteCarlo/%getcovariance.cpp
*/

// $Id$

#include "ql/MonteCarlo/getcovariance.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        using QuantLib::Math::Matrix;

        Matrix getCovariance(const Array &volatilities,
                             const Matrix &correlations){
            unsigned int size = volatilities.size();
            QL_REQUIRE(correlations.rows() == size,
                       "getCovariance: volatilities and correlations "
                       "have different size");
            QL_REQUIRE(correlations.columns() == size,
                "getCovariance: correlation matrix is not square");

            Matrix covariance(size,size);
            for(unsigned int i = 0; i < size; i++){
                for(unsigned int j = 0; j < i; j++){
                    covariance[i][j] = volatilities[i] * volatilities[j] *
                            0.5 * (correlations[i][j] + correlations[j][i]);
                    covariance[j][i] = covariance[i][j];
                }
                covariance[i][i] = volatilities[i] * volatilities[i];
            }
            return covariance;
        }

    }

}


/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file getcovariance.cpp

    $Source$
    $Log$
    Revision 1.4  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.3  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.2  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/15 13:48:00  marmar
    getCovariance function added

*/

#include "MonteCarlo/getcovariance.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        using QuantLib::Math::Matrix;

        Matrix getCovariance(const Array &volatilities,
                             const Matrix &correlations){
            int size = volatilities.size();
            QL_REQUIRE(correlations.rows() == size,
                       "getCovariance: volatilities and correlations "
                       "have different size");
            QL_REQUIRE(correlations.columns() == size,
                "getCovariance: correlation matrix is not square");

            Matrix covariance(size,size);
            for(int i = 0; i < size; i++){
                for(int j = 0; j < i; j++){
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

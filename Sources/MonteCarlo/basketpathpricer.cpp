
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

/*! \file basketpathpricer.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/04 13:32:54  enri
    tons of typos fixed

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

    Revision 1.1  2001/02/02 10:53:18  marmar
    Example of a path pricer depending on multiple factors

*/

#include "MonteCarlo/basketpathpricer.hpp"
#include "qlerrors.hpp"
#include "dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        BasketPathPricer::BasketPathPricer(const Array &underlying,
            double discount) : underlying_(underlying), discount_(discount) {
            QL_REQUIRE(discount_ > 0.0,
                "SinglePathEuropeanPricer: discount must be positive");
            isInitialized_ = true;
        }

        double BasketPathPricer::value(const MultiPath & path) const {
            int numAssets = path.rows(), numSteps = path.columns();
            QL_REQUIRE(isInitialized_,
                "BasketPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "BasketPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double maxPrice = -QL_MAX_DOUBLE;
            for(int i = 0; i < numAssets; i++){
                double price = underlying_[i];
                for(int j = 0; j < numSteps; j++)
                    price *= QL_EXP(path[i][j]);
                maxPrice = QL_MAX(maxPrice, price);
            }
            return discount_*maxPrice;  //This is the GOOD one!!
        }

    }

}

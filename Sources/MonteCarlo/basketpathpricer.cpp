
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
    $Log$
    Revision 1.8  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#include "ql/MonteCarlo/basketpathpricer.hpp"
#include "ql/dataformatters.hpp"

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


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

/*! \file basketpathpricer.cpp
    \brief multipath pricer for European-type basket option

    \fullpath
    MonteCarlo/%basketpathpricer.cpp

*/

// $Id$

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

        double BasketPathPricer::operator()(const MultiPath & path) const {
            unsigned int numAssets = path.rows(), numSteps = path.columns();
            QL_REQUIRE(isInitialized_,
                "BasketPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "BasketPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double maxPrice = -QL_MAX_DOUBLE;
            for(unsigned int i = 0; i < numAssets; i++){
                double price = underlying_[i];
                for(unsigned int j = 0; j < numSteps; j++)
                    price *= QL_EXP(path[i][j]);
                maxPrice = QL_MAX(maxPrice, price);
            }
            return discount_*maxPrice;  //This is the GOOD one!!
        }

    }

}

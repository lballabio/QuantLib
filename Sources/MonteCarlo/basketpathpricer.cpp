
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
    \fullpath Sources/MonteCarlo/%basketpathpricer.cpp
    \brief multipath pricer for European-type basket option

*/

// $Id$
// $Log$
// Revision 1.14  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.13  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.9  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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

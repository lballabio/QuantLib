
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

/*! \file singleassetpathpricer.cpp
    \brief generic single asset %path pricer

    \fullpath
    ql/MonteCarlo/%singleassetpathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/singleassetpathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! generic %path pricer for single asset
        SingleAssetPathPricer::SingleAssetPathPricer(Option::Type type,
            double underlying, double strike, double discount,
            bool antitheticVariance)
            : type_(type), underlying_(underlying), strike_(strike),
              discount_(discount),
              antitheticVariance_(antitheticVariance) {
            QL_REQUIRE(strike_ > 0.0,
                "SingleAssetPathPricer: strike must be positive");
            QL_REQUIRE(underlying_ > 0.0,
                "SingleAssetPathPricer: underlying must be positive");
            QL_REQUIRE(discount_ > 0.0,
                "SingleAssetPathPricer: discount must be positive");
        }

    }

}


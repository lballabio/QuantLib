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
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file everestpathpricer.cpp

    $Sourc$
    $Log$
    Revision 1.1  2001/03/06 16:59:31  marmar
    First, simplified version, of everest option

*/

#include "everestpathpricer.h"
#include "qlerrors.h"
#include "dataformatters.h"

namespace QuantLib {

    namespace MonteCarlo {

        EverestPathPricer::EverestPathPricer(const Array &underlying,
            double discount): underlying_(underlying), discount_(discount) {
            QL_REQUIRE(discount_ > 0.0,
                "EverestPathPricer: discount must be positive");
            isInitialized_ = true;
        }

        double EverestPathPricer::value(const MultiPath & path) const {
            int numAssets = path.rows();
            QL_REQUIRE(isInitialized_,
                "EverestPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "EverestPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");
            
            double minPrice = QL_MAX_DOUBLE;
            for(int i = 0; i < numAssets; i++)
                minPrice = QL_MIN(minPrice, QL_EXP(path[i][0]));

            return discount_ * minPrice;  
        }

    }

}

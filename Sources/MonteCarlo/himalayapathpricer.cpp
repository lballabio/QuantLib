/*
 * Copyright (C) 2000
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

/*! \file himalayapathpricer.cpp

    $Source$
    $Log$
    Revision 1.2  2001/03/06 15:13:36  marmar
    Himalaya option now can handle any number of time steps

    Revision 1.1  2001/02/07 10:13:53  marmar
    Himalaya-type path pricer

*/

#include "himalayapathpricer.h"
#include "qlerrors.h"
#include "dataformatters.h"

namespace QuantLib {

    namespace MonteCarlo {

        HimalayaPathPricer::HimalayaPathPricer(const Array &underlying,
            double strike, double discount)
            : underlying_(underlying), strike_(strike), discount_(discount) {
            QL_REQUIRE(discount_ > 0.0,
                "SinglePathEuropeanPricer: discount must be positive");
            isInitialized_ = true;
        }

        double HimalayaPathPricer::value(const MultiPath & path) const {
            int numAssets = path.rows(), numSteps = path.columns();
            QL_REQUIRE(isInitialized_,
                "HimalayaPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "HimalayaPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");


            Array prices(underlying_);
            double averagePrice = 0;
            std::vector<bool> remainingAssets(numAssets, true);

            for(int j = 0; j < numSteps; j++){
                double bestPrice = 0.0;
                int removeAsset;
                for(int i = 0; i < numAssets; i++){
                    if(remainingAssets[i]){
                        prices[i] *= QL_EXP(path[i][j]);
                        if(prices[i] >= bestPrice) {
                            bestPrice = prices[i];
                            removeAsset = i;
                        }
                    }
                }
                remainingAssets[removeAsset] = false;
                averagePrice += bestPrice;
            }
            averagePrice /= QL_MIN(numSteps, numAssets);
            double optPrice = QL_MAX(averagePrice - strike_, 0.0);
            return discount_ * optPrice;
        }

    }

}

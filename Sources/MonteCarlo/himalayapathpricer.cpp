
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file himalayapathpricer.cpp
    \brief multipath pricer for European-type Himalaya option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.9  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.8  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/himalayapathpricer.hpp"
#include "ql/dataformatters.hpp"

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


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

/*! \file himalayapathpricer.cpp
    \brief multipath pricer for European-type Himalaya option

    \fullpath
    ql/MonteCarlo/%himalayapathpricer.cpp
*/

// $Id$

#include <ql/MonteCarlo/himalayapathpricer.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        HimalayaPathPricer::HimalayaPathPricer(const Array& underlying,
            double strike,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying), strike_(strike) {
            for (Size j=0; j<underlying_.size(); j++) {
                QL_REQUIRE(underlying_[j]>0.0,
                    "HimalayaPathPricer: "
                    "underlying less/equal zero not allowed");
            QL_REQUIRE(strike>0.0,
                "HimalayaPathPricer: "
                "strike less/equal zero not allowed");
            }
        }

        double HimalayaPathPricer::operator()(const MultiPath& multiPath)
          const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "HimalayaPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");
            QL_REQUIRE(numAssets>0,
                "HimalayaPathPricer: no asset given");


            Array prices(underlying_);
            double averagePrice = 0;
            std::vector<bool> remainingAssets(numAssets, true);
            double bestPrice;
            Size removeAsset, i, j;
            for(i = 0; i < numSteps; i++) {
                bestPrice = 0.0;
                // dummy assignement to avoid compiler warning
                removeAsset=0;
                for(j = 0; j < numAssets; j++) {
                    if(remainingAssets[j]) {
                        prices[j] *= QL_EXP(multiPath[j].drift()[i]+
                            multiPath[j].diffusion()[i]);
                        if(prices[j] >= bestPrice) {
                            bestPrice = prices[j];
                            removeAsset = j;
                        }
                    }
                }
                remainingAssets[removeAsset] = false;
                averagePrice += bestPrice;
            }
            averagePrice /= QL_MIN(numSteps, numAssets);
            double optPrice = QL_MAX(averagePrice - strike_, 0.0);

            if (useAntitheticVariance_) {
                prices = underlying_;
                averagePrice = 0;
                remainingAssets = std::vector<bool>(numAssets, true);
                for(i = 0; i < numSteps; i++) {
                    bestPrice = 0.0;
                    // dummy assignement to avoid compiler warning
                    removeAsset=0;
                    for(j = 0; j < numAssets; j++) {
                        if(remainingAssets[j]) {
                            prices[j] *= QL_EXP(multiPath[j].drift()[i]-
                                multiPath[j].diffusion()[i]);
                            if(prices[j] >= bestPrice) {
                                bestPrice = prices[j];
                                removeAsset = j;
                            }
                        }
                    }
                    remainingAssets[removeAsset] = false;
                    averagePrice += bestPrice;
                }
                averagePrice /= QL_MIN(numSteps, numAssets);
                double optPrice2 = QL_MAX(averagePrice - strike_, 0.0);

                return discount_ * 0.5 * (optPrice+optPrice2);
            } else {
                return discount_ * optPrice;
            }

        }

    }

}


/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file maxbasketpathpricer.cpp
    \brief multipath pricer for max basket option

    \fullpath
    MonteCarlo/%basketpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/maxbasketpathpricer.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        MaxBasketPathPricer::MaxBasketPathPricer(const Array& underlying,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying) {
            for (Size i=0; i<underlying_.size(); i++) {
                QL_REQUIRE(underlying_[i]>0.0,
                    "MaxBasketPathPricer: "
                    "underlying less/equal zero not allowed");
            }
        }

        double MaxBasketPathPricer::operator()(const MultiPath& multiPath)
          const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "MaxBasketPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double log_drift, log_diffusion;
            Size i,j;
            if (useAntitheticVariance_) {
                double maxPrice = -QL_MAX_DOUBLE, maxPrice2 = -QL_MAX_DOUBLE;
                for(j = 0; j < numAssets; j++) {
                    log_drift = log_diffusion = 0.0;
                    for(i = 0; i < numSteps; i++) {
                        log_drift += multiPath[j].drift()[i];
                        log_diffusion += multiPath[j].diffusion()[i];
                    }
                    maxPrice = QL_MAX(maxPrice,
                        underlying_[j]*QL_EXP(log_drift+log_diffusion));
                    maxPrice2 = QL_MAX(maxPrice2,
                        underlying_[j]*QL_EXP(log_drift-log_diffusion));
                }
                return discount_*0.5*(maxPrice+maxPrice2);
            } else {
                double maxPrice = -QL_MAX_DOUBLE;
                for(j = 0; j < numAssets; j++) {
                    log_drift = log_diffusion = 0.0;
                    for(i = 0; i < numSteps; i++) {
                        log_drift += multiPath[j].drift()[i];
                        log_diffusion += multiPath[j].diffusion()[i];
                    }
                    maxPrice = QL_MAX(maxPrice,
                        underlying_[j]*QL_EXP(log_drift+log_diffusion));
                }
                return discount_*maxPrice;
            }

        }

    }

}



/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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

        MaxBasketPathPricer_old::MaxBasketPathPricer_old(const Array& underlying,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying) {
            for (Size i=0; i<underlying_.size(); i++) {
                QL_REQUIRE(underlying_[i]>0.0,
                    "MaxBasketPathPricer_old: "
                    "underlying less/equal zero not allowed");
            }
        }

        double MaxBasketPathPricer_old::operator()(const MultiPath& multiPath)
          const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "MaxBasketPathPricer_old: the multi-path must contain "
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

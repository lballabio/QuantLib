
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file everestpathpricer.cpp
    \brief path pricer for European-type Everest option
*/

#include <ql/MonteCarlo/everestpathpricer.hpp>

namespace QuantLib {

    EverestPathPricer_old::EverestPathPricer_old(DiscountFactor discount,
                                                 bool useAntitheticVariance)
    : PathPricer_old<MultiPath>(discount, useAntitheticVariance) {}


    double EverestPathPricer_old::operator()(const MultiPath& multiPath) 
                                                                       const {
        Size numAssets = multiPath.assetNumber();
        Size numSteps = multiPath.pathSize();

        double log_drift, log_diffusion;
        Size i,j;
        if (useAntitheticVariance_) {
            double minPrice = QL_MAX_DOUBLE, minPrice2 = QL_MAX_DOUBLE;
            for( j = 0; j < numAssets; j++) {
                log_drift = log_diffusion = 0.0;
                for( i = 0; i < numSteps; i++) {
                    log_drift += multiPath[j].drift()[i];
                    log_diffusion += multiPath[j].diffusion()[i];
                }
                minPrice  = QL_MIN(minPrice,  QL_EXP(log_drift+log_diffusion));
                minPrice2 = QL_MIN(minPrice2, QL_EXP(log_drift-log_diffusion));
            }

            return discount_ * 0.5 * (minPrice+minPrice2);
        } else {
            double minPrice = QL_MAX_DOUBLE;
            for( j = 0; j < numAssets; j++) {
                log_drift = log_diffusion = 0.0;
                for( i = 0; i < numSteps; i++) {
                    log_drift += multiPath[j].drift()[i];
                    log_diffusion += multiPath[j].diffusion()[i];
                }
                minPrice = QL_MIN(minPrice, QL_EXP(log_drift+log_diffusion));
            }

            return discount_ * minPrice;
        }
    }

}

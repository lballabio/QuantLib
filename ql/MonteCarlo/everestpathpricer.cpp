
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
/*! \file everestpathpricer.cpp
    \brief path pricer for European-type Everest option

    \fullpath
    MonteCarlo/%everestpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/everestpathpricer.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        EverestPathPricer::EverestPathPricer(DiscountFactor discount,
            bool useAntitheticVariance)
        : PathPricer<MultiPath>(discount, useAntitheticVariance) {}


        double EverestPathPricer::operator()(const MultiPath& multiPath) const {
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

}

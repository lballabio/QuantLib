
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
    \brief multipath pricer for European-type basket option

    \fullpath
    MonteCarlo/%basketpathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/basketpathpricer.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        BasketPathPricer::BasketPathPricer(const Array& underlying,
            double discount, bool antitheticVariance)
        : underlying_(underlying), discount_(discount),
          antitheticVariance_(antitheticVariance) {
            QL_REQUIRE(discount_ > 0.0,
                "BasketPathPricer: discount must be positive");
            isInitialized_ = true;
        }

        double BasketPathPricer::operator()(const MultiPath & multiPath) const {
            unsigned int numAssets = multiPath.assetNumber();
            unsigned int numSteps = multiPath.pathSize();
            QL_REQUIRE(isInitialized_,
                "BasketPathPricer: pricer not initialized");
            QL_REQUIRE(underlying_.size() == numAssets,
                "BasketPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double log_drift, log_diffusion;
            if (antitheticVariance_) {
                double maxPrice = -QL_MAX_DOUBLE, maxPrice2 = -QL_MAX_DOUBLE;
                for(unsigned int j = 0; j < numAssets; j++) {
                    log_drift = log_diffusion = 0.0;
                    for(unsigned int i = 0; i < numSteps; i++) {
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
                for(unsigned int j = 0; j < numAssets; j++) {
                    log_drift = log_diffusion = 0.0;
                    for(unsigned int i = 0; i < numSteps; i++) {
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

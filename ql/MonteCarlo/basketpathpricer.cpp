
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

#include <ql/MonteCarlo/basketpathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>
#include <ql/dataformatters.hpp>

using QuantLib::Pricers::ExercisePayoff;

namespace QuantLib {

    namespace MonteCarlo {

        BasketPathPricer::BasketPathPricer(Option::Type type,
            const Array& underlying, double strike,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer<MultiPath>(discount, useAntitheticVariance), type_(type),
          underlying_(underlying) {
            for (Size j=0; j<underlying_.size(); j++) {
                QL_REQUIRE(underlying_[j]>0.0,
                    "BasketPathPricer: "
                    "underlying less/equal zero not allowed");
            QL_REQUIRE(strike>0.0,
                "BasketPathPricer: "
                "strike less/equal zero not allowed");
            }
        }

        double BasketPathPricer::operator()(const MultiPath& multiPath)
          const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "BasketPathPricer: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            double log_drift=0.0, log_diffusion=0.0;
            Size i,j;
            double basketPrice = 0.0;
            for(j = 0; j < numAssets; j++) {
                log_drift = log_diffusion = 0.0;
                for(i = 0; i < numSteps; i++) {
                    log_drift += multiPath[j].drift()[i];
                    log_diffusion += multiPath[j].diffusion()[i];
                }
                basketPrice += underlying_[j]*
                    QL_EXP(log_drift+log_diffusion);
            }
            if (useAntitheticVariance_) {
                double basketPrice2 = 0.0;
                for(j = 0; j < numAssets; j++) {
                    basketPrice2 += underlying_[j]*
                        QL_EXP(log_drift-log_diffusion);
                }
                return discount_*0.5*
                    (ExercisePayoff(type_, basketPrice, strike_)+
                    ExercisePayoff(type_, basketPrice2, strike_));
            } else {
                return discount_*ExercisePayoff(type_, basketPrice, strike_);
            }

        }

    }

}

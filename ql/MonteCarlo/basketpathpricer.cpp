

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file basketpathpricer.cpp
    \brief multipath pricer for European-type basket option

    \fullpath
    MonteCarlo/%basketpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/basketpathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

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

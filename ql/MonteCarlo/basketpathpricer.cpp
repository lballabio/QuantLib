
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
/*! \file basketpathpricer.cpp
    \brief multipath pricer for European-type basket option

    \fullpath
    MonteCarlo/%basketpathpricer.cpp

*/

// $Id$

#include <ql/MonteCarlo/basketpathpricer.hpp>
#include <ql/Pricers/singleassetoption.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        BasketPathPricer_old::BasketPathPricer_old(Option::Type type,
            const std::vector<double>& underlying, double strike,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying), payoff_(type, strike) {
            for (Size j=0; j<underlying_.size(); j++) {
                QL_REQUIRE(underlying_[j]>0.0,
                    "BasketPathPricer_old: "
                    "underlying less/equal zero not allowed");
            QL_REQUIRE(strike>0.0,
                "BasketPathPricer_old: "
                "strike less/equal zero not allowed");
            }
        }

        double BasketPathPricer_old::operator()(const MultiPath& multiPath)
          const {

            Size numSteps = multiPath.pathSize();
            Size numAssets = multiPath.assetNumber();
            QL_REQUIRE(underlying_.size() == numAssets,
                "BasketPathPricer_old: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");

            QL_REQUIRE(numSteps>0,
                "BasketPathPricer_old: the path cannot be empty");

            // start the simulation
            std::vector<double> log_drift(numAssets, 0.0);
            std::vector<double> log_diffusion(numAssets, 0.0);
            Size i,j;
            double basketPrice = 0.0;
            for(j = 0; j < numAssets; j++) {
                log_drift[j] = log_diffusion[j] = 0.0;
                for(i = 0; i < numSteps; i++) {
                    log_drift[j] += multiPath[j].drift()[i];
                    log_diffusion[j] += multiPath[j].diffusion()[i];
                }
                basketPrice += underlying_[j]*
                    QL_EXP(log_drift[j]+log_diffusion[j]);
            }
            if (useAntitheticVariance_) {
                double basketPrice2 = 0.0;
                for(j = 0; j < numAssets; j++) {
                    basketPrice2 += underlying_[j]*
                        QL_EXP(log_drift[j]-log_diffusion[j]);
                }
                return discount_ * 0.5 *
                    (payoff_(basketPrice) + payoff_(basketPrice2));
            } else {
                return discount_ * payoff_(basketPrice);
            }

        }

    }

}

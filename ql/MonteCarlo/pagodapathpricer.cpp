
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

/*! \file pagodapathpricer.cpp
    \brief path pricer for pagoda options
*/

#include <ql/MonteCarlo/pagodapathpricer.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        PagodaPathPricer_old::PagodaPathPricer_old(const std::vector<double>& underlying,
            double roof,
            DiscountFactor discount, bool useAntitheticVariance)
        : PathPricer_old<MultiPath>(discount, useAntitheticVariance),
          underlying_(underlying), roof_(roof) {}

        double PagodaPathPricer_old::operator()(const MultiPath& multiPath) const {
            Size numAssets = multiPath.assetNumber();
            Size numSteps = multiPath.pathSize();
            QL_REQUIRE(underlying_.size() == numAssets,
                "PagodaPathPricer_old: the multi-path must contain "
                + IntegerFormatter::toString(underlying_.size()) +" assets");


            Size i,j;
            if (useAntitheticVariance_) {
                double averageGain = 0.0, averageGain2 = 0.0;
                for(i = 0; i < numSteps; i++)
                    for(j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]+
                                    multiPath[j].diffusion()[i])
                             -1.0);
                        averageGain2 += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]-
                                    multiPath[j].diffusion()[i])
                             -1.0);
                    }
                return discount_ * 0.5 *
                    (QL_MAX(0.0, QL_MIN(roof_, averageGain))+
                     QL_MAX(0.0, QL_MIN(roof_, averageGain2)));
            } else {
                double averageGain = 0.0;
                for(i = 0; i < numSteps; i++)
                    for(j = 0; j < numAssets; j++) {
                        averageGain += underlying_[j] *
                            (QL_EXP(multiPath[j].drift()[i]+
                                    multiPath[j].diffusion()[i])
                             -1.0);
                    }
                return discount_ * QL_MAX(0.0, QL_MIN(roof_, averageGain));
            }

        }

    }

}

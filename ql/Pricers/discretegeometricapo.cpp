
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

/*! \file discretegeometricapo.cpp
    \brief Discrete Geometric Average Price Option

    \fullpath
    ql/Pricers/%discretegeometricapo.cpp
*/

// $Id$

#include "ql/Pricers/discretegeometricapo.hpp"
#include <iostream>
#include <numeric>

namespace QuantLib {

    namespace Pricers {

        const Math::CumulativeNormalDistribution DiscreteGeometricAPO::f_;

        DiscreteGeometricAPO::DiscreteGeometricAPO(Option::Type type,
            double underlying, double strike, Spread dividendYield,
            Rate riskFreeRate, const std::vector<Time>& times,
            double volatility)
        : times_(times), SingleAssetOption(type, underlying, strike,
          dividendYield, riskFreeRate, times.back(), volatility) {}

        double DiscreteGeometricAPO::value() const {
            // almost ready for mid-life re-evaluation
            std::vector<double> pastFixings(0, 0.0);
            double runningAverage = std::accumulate(pastFixings.begin(),
                pastFixings.end(), 1.0, std::multiplies<double>());
            size_t m = pastFixings.size();
            double runningLogAverage = QL_LOG(runningAverage);
            QL_REQUIRE(runningLogAverage==0.0, "not zero");

            double N = double(times_.size()+m);
            double pastWeight = m/N;
            double futureWeight = 1-pastWeight;
            QL_REQUIRE(futureWeight==1.0, "not one");

            double nu = riskFreeRate_ - dividendYield_ -
                0.5*volatility_*volatility_;
            double muG = pastWeight * runningLogAverage +
                futureWeight * QL_LOG(underlying_) +
                nu/N*std::accumulate(times_.begin(), times_.end(), 0.0);

            size_t i;
            double temp = 0.0;
            for (i=m+1; i<N; i++)
                temp += times_[i-m-1]*(N-i);
            double sigmaG_2 = volatility_*volatility_ /N/N * (
                std::accumulate(times_.begin(), times_.end(), 0.0)+
                2.0*temp
                );

            double x1 = (muG-QL_LOG(strike_)+sigmaG_2)/QL_SQRT(sigmaG_2);
            double x2 = x1-QL_SQRT(sigmaG_2);

            switch (type_) {
                case Option::Call:
                    return QL_EXP(-riskFreeRate_*residualTime_)*
                (QL_EXP(muG + sigmaG_2 / 2.0) * f_(x1) -
                strike_ * f_(x2));
                    break;
                case Option::Put:
                    return QL_EXP(-riskFreeRate_*residualTime_)*
                        (strike_ * f_(-x2) -
                        QL_EXP(muG + sigmaG_2 / 2.0) * f_(-x1)
                        );
                    break;
                default:
                    throw IllegalArgumentError(
                        "DiscreteGeometricAPO: invalid option type");
            }


        }
    }
}


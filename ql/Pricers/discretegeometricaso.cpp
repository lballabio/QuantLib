
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

/*! \file discretegeometricaso.cpp
    \brief Discrete Geometric Average Strike Option
*/

#include <ql/Pricers/discretegeometricaso.hpp>
#include <numeric>

namespace QuantLib {

    const CumulativeNormalDistribution DiscreteGeometricASO::f_;

    DiscreteGeometricASO::DiscreteGeometricASO(
                   Option::Type type, double underlying, Spread dividendYield,
                   Rate riskFreeRate, const std::vector<Time>& times,
                   double volatility)
    : SingleAssetOption(type, underlying, underlying, dividendYield,
                        riskFreeRate, times.back(), volatility), 
      times_(times) {}

    double DiscreteGeometricASO::value() const {
        // almost ready for mid-life re-evaluation
        std::vector<double> pastFixings(0, 0.0);
        double runningAverage = std::accumulate(pastFixings.begin(),
                                                pastFixings.end(), 1.0, 
                                                std::multiplies<double>());
        Size m = pastFixings.size();
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

        Size i;
        double temp = 0.0;
        for (i=m+1; i<N; i++)
            temp += times_[i-m-1]*(N-i);
        double sigmaG_2 = volatility_*volatility_ /N/N * 
            (std::accumulate(times_.begin(), times_.end(), 0.0)+ 2.0*temp);

        double covarianceTerm = volatility_*volatility_/N *
            std::accumulate(times_.begin(), times_.end(), 0.0);
        double sigmaSum_2 = sigmaG_2+
            volatility_*volatility_*residualTime_-
            2.0*covarianceTerm;
        double y1 = (QL_LOG(underlying_)+
                     (riskFreeRate_-dividendYield_)*residualTime_-
                     muG - sigmaG_2/2.0 + sigmaSum_2/2.0)
            /QL_SQRT(sigmaSum_2);
        double y2=y1-QL_SQRT(sigmaSum_2);

        double result;
        switch (payoff_.optionType()) {
          case Option::Call:
            result = underlying_*QL_EXP(-dividendYield_*residualTime_)
                *f_(y1)-
                QL_EXP(muG+sigmaG_2/2.0-riskFreeRate_*residualTime_)
                *f_(y2);
            break;
          case Option::Put:
            result = -underlying_*QL_EXP(-dividendYield_*residualTime_)
                *f_(-y1)+
                QL_EXP(muG+sigmaG_2/2.0-riskFreeRate_*residualTime_)
                *f_(-y2);
            break;
          default:
            QL_FAIL("DiscreteGeometricASO: "
                    "invalid option type");
        }
        return result;
    }

}


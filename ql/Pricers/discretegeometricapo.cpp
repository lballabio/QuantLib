
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

#include <ql/Pricers/discretegeometricapo.hpp>
#include <numeric>

namespace QuantLib {

    const CumulativeNormalDistribution DiscreteGeometricAPO::f_;

    DiscreteGeometricAPO::DiscreteGeometricAPO(
                       Option::Type type, double underlying, double strike, 
                       Spread dividendYield, Rate riskFreeRate, 
                       const std::vector<Time>& times, double volatility)
    : SingleAssetOption(type, underlying, strike, dividendYield,
                        riskFreeRate, times.back(), volatility), 
      times_(times) {}

    double DiscreteGeometricAPO::value() const {
        // almost ready for mid-life re-evaluation
        std::vector<double> pastFixings(0, 0.0);
        double runningAverage = std::accumulate(pastFixings.begin(),
                                                pastFixings.end(), 1.0, 
                                                std::multiplies<double>());
        Size m = pastFixings.size();
        double runningLogAverage = QL_LOG(runningAverage);

        double N = double(times_.size()+m);
        double pastWeight = m/N;
        double futureWeight = 1-pastWeight;

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

        double x1 = (muG-QL_LOG(payoff_.strike())+sigmaG_2)/QL_SQRT(sigmaG_2);
        double x2 = x1-QL_SQRT(sigmaG_2);

        double result;
        switch (payoff_.optionType()) {
          case Option::Call:
            result = QL_EXP(-riskFreeRate_*residualTime_)*
                (QL_EXP(muG + sigmaG_2 / 2.0) * f_(x1) -
                 payoff_.strike() * f_(x2));
            break;
          case Option::Put:
            result = QL_EXP(-riskFreeRate_*residualTime_)*
                (payoff_.strike() * f_(-x2) -
                 QL_EXP(muG + sigmaG_2 / 2.0) * f_(-x1)
                 );
            break;
          default:
            QL_FAIL("DiscreteGeometricAPO: "
                    "invalid option type");
        }
        return result;
    }

}



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

#include <ql/Pricers/discretegeometricaso.hpp>
#include <numeric>

namespace QuantLib {

    const CumulativeNormalDistribution DiscreteGeometricASO::f_;

    DiscreteGeometricASO::DiscreteGeometricASO(
                   Option::Type type, Real underlying, Spread dividendYield,
                   Rate riskFreeRate, const std::vector<Time>& times,
                   Volatility volatility)
    : SingleAssetOption(type, underlying, underlying, dividendYield,
                        riskFreeRate, times.back(), volatility), 
      times_(times) {}

    Real DiscreteGeometricASO::value() const {
        // almost ready for mid-life re-evaluation
        std::vector<Real> pastFixings(0, 0.0);
        Real runningAverage = std::accumulate(pastFixings.begin(),
                                              pastFixings.end(), 1.0, 
                                              std::multiplies<Real>());
        Size m = pastFixings.size();
        Real runningLogAverage = QL_LOG(runningAverage);
        QL_REQUIRE(runningLogAverage==0.0, "not zero");

        Real N = Real(times_.size()+m);
        Real pastWeight = m/N;
        Real futureWeight = 1-pastWeight;
        QL_REQUIRE(futureWeight==1.0, "not one");

        Rate nu = riskFreeRate_ - dividendYield_ -
            0.5*volatility_*volatility_;
        Real muG = pastWeight * runningLogAverage +
            futureWeight * QL_LOG(underlying_) +
            nu/N*std::accumulate(times_.begin(), times_.end(), 0.0);

        Size i;
        Real temp = 0.0;
        for (i=m+1; i<N; i++)
            temp += times_[i-m-1]*(N-i);
        Real sigmaG_2 = volatility_*volatility_ /N/N * 
            (std::accumulate(times_.begin(), times_.end(), 0.0)+ 2.0*temp);

        Real covarianceTerm = volatility_*volatility_/N *
            std::accumulate(times_.begin(), times_.end(), 0.0);
        Real sigmaSum_2 = sigmaG_2+
            volatility_*volatility_*residualTime_-
            2.0*covarianceTerm;
        Real y1 = (QL_LOG(underlying_)+
                     (riskFreeRate_-dividendYield_)*residualTime_-
                     muG - sigmaG_2/2.0 + sigmaSum_2/2.0)
            /QL_SQRT(sigmaSum_2);
        Real y2=y1-QL_SQRT(sigmaSum_2);

        Real result;
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
            QL_FAIL("invalid option type");
        }
        return result;
    }

}


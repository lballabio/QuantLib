
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

#include <ql/Pricers/mceverest.hpp>

namespace QuantLib {

    namespace {

        class EverestPathPricer_old : public PathPricer_old<MultiPath> {
          public:
            EverestPathPricer_old(DiscountFactor discount,
                                  bool useAntitheticVariance)
            : PathPricer_old<MultiPath>(discount, useAntitheticVariance) {}

            double operator()(const MultiPath& multiPath) const {
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
                        minPrice  = QL_MIN(minPrice,
                                           QL_EXP(log_drift+log_diffusion));
                        minPrice2 = QL_MIN(minPrice2, 
                                           QL_EXP(log_drift-log_diffusion));
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
                        minPrice = QL_MIN(minPrice, 
                                          QL_EXP(log_drift+log_diffusion));
                    }

                    return discount_ * minPrice;
                }
            }
        };

    }

    McEverest::McEverest(const Array& dividendYield,
                         const Matrix& covariance,
                         Rate riskFreeRate, Time residualTime,
                         bool antitheticVariance, long seed) {

        Size  n = covariance.rows();
        QL_REQUIRE(covariance.columns() == n,
                   "McEverest: covariance matrix not square");
        QL_REQUIRE(dividendYield.size() == n,
                   "McEverest: dividendYield size does not match"
                   " that of covariance matrix");
        QL_REQUIRE(residualTime > 0,
                   "McEverest: residualTime must be positive");

        // initialize the path generator
        Array mu(riskFreeRate - dividendYield
                 - 0.5 * covariance.diagonal());

        boost::shared_ptr<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(mu, covariance,
                                           TimeGrid(residualTime, 1), seed));

        // initialize the pricer on the path pricer
        boost::shared_ptr<PathPricer_old<MultiPath> > pathPricer(
            new EverestPathPricer_old(QL_EXP(-riskFreeRate*residualTime),
            antitheticVariance));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset_old<
                                          PseudoRandomSequence_old> > > (
            new MonteCarloModel<MultiAsset_old<
                                PseudoRandomSequence_old> > (
            pathGenerator, pathPricer, Statistics(), false));

    }

}


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

#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/Pricers/mchimalaya.hpp>

namespace QuantLib {

    namespace {

        class HimalayaPathPricer_old : public PathPricer_old<MultiPath> {
          public:
            HimalayaPathPricer_old(const std::vector<double>& underlying,
                                   double strike,
                                   DiscountFactor discount,
                                   bool useAntitheticVariance)
            : PathPricer_old<MultiPath>(discount, useAntitheticVariance),
              underlying_(underlying), strike_(strike) {
                for (Size j=0; j<underlying_.size(); j++) {
                    QL_REQUIRE(underlying_[j]>0.0,
                               "HimalayaPathPricer_old: "
                               "underlying less/equal zero not allowed");
                    QL_REQUIRE(strike>=0.0,
                               "HimalayaPathPricer_old: "
                               "strike less than zero not allowed");
                }
            }

            double operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "HimalayaPathPricer_old: "
                           "the multi-path must contain "
                           + IntegerFormatter::toString(underlying_.size()) + 
                           " assets");
                QL_REQUIRE(numAssets>0,
                           "HimalayaPathPricer_old: no asset given");

                std::vector<double> prices(underlying_);
                double averagePrice = 0;
                std::vector<bool> remainingAssets(numAssets, true);
                double bestPrice;
                Size removeAsset, i, j;
                Size fixings = numSteps;
                if (multiPath[0].timeGrid().mandatoryTimes()[0] == 0.0) {
                    bestPrice = 0.0;
                    // dummy assignement to avoid compiler warning
                    removeAsset=0;
                    for (j = 0; j < numAssets; j++) {
                        if (prices[j] >= bestPrice) {
                            bestPrice = prices[j];
                            removeAsset = j;
                        }
                    }
                    remainingAssets[removeAsset] = false;
                    averagePrice += bestPrice;
                    fixings = numSteps+1;
                }
                for (i = 0; i < numSteps; i++) {
                    bestPrice = 0.0;
                    // dummy assignement to avoid compiler warning
                    removeAsset=0;
                    for (j = 0; j < numAssets; j++) {
                        if (remainingAssets[j]) {
                            prices[j] *= QL_EXP(multiPath[j].drift()[i]+
                                                multiPath[j].diffusion()[i]);
                            if (prices[j] >= bestPrice) {
                                bestPrice = prices[j];
                                removeAsset = j;
                            }
                        }
                    }
                    remainingAssets[removeAsset] = false;
                    averagePrice += bestPrice;
                }
                averagePrice /= QL_MIN(fixings, numAssets);
                double optPrice = QL_MAX(averagePrice - strike_, 0.0);

                if (useAntitheticVariance_) {
                    prices = underlying_;
                    averagePrice = 0;
                    remainingAssets = std::vector<bool>(numAssets, true);
                    if (multiPath[0].timeGrid().mandatoryTimes()[0] == 0.0) {
                        bestPrice = 0.0;
                        // dummy assignement to avoid compiler warning
                        removeAsset=0;
                        for (j = 0; j < numAssets; j++) {
                            if (prices[j] >= bestPrice) {
                                bestPrice = prices[j];
                                removeAsset = j;
                            }
                        }
                        remainingAssets[removeAsset] = false;
                        averagePrice += bestPrice;
                    }
                    for (i = 0; i < numSteps; i++) {
                        bestPrice = 0.0;
                        // dummy assignement to avoid compiler warning
                        removeAsset=0;
                        for (j = 0; j < numAssets; j++) {
                            if (remainingAssets[j]) {
                                prices[j] *= 
                                    QL_EXP(multiPath[j].drift()[i] -
                                           multiPath[j].diffusion()[i]);
                                if (prices[j] >= bestPrice) {
                                    bestPrice = prices[j];
                                    removeAsset = j;
                                }
                            }
                        }
                        remainingAssets[removeAsset] = false;
                        averagePrice += bestPrice;
                    }
                    averagePrice /= QL_MIN(fixings, numAssets);
                    double optPrice2 = QL_MAX(averagePrice - strike_, 0.0);

                    return discount_ * 0.5 * (optPrice+optPrice2);
                } else {
                    return discount_ * optPrice;
                }
            }

          private:
            Option::Type type_;
            std::vector<double> underlying_;
            double strike_;
        };

    }

    McHimalaya::McHimalaya(const std::vector<double>& underlying,
                           const Array& dividendYield, 
                           const Matrix& covariance,
                           Rate riskFreeRate, double strike,
                           const std::vector<Time>& times,
                           bool antitheticVariance, long seed) {

        Size  n = covariance.rows();
        QL_REQUIRE(covariance.columns() == n,
                   "McHimalaya: covariance matrix not square");
        QL_REQUIRE(underlying.size() == n,
                   "McHimalaya: underlying size does not match that of"
                   " covariance matrix");
        QL_REQUIRE(dividendYield.size() == n,
                   "McHimalaya: dividendYield size does not match"
                   " that of covariance matrix");
        QL_REQUIRE(times.size() >= 1,
                   "McHimalaya: you must have at least one time-step");

        // initialize the path generator
        Array mu(riskFreeRate - dividendYield
                 - 0.5 * covariance.diagonal());

        Handle<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(
                mu, covariance,
                TimeGrid(times.begin(), times.end()),
                seed));
        double residualTime = times[times.size()-1];

        // initialize the pricer on the path pricer
        Handle<PathPricer_old<MultiPath> > pathPricer(
            new HimalayaPathPricer_old(
                underlying, strike, QL_EXP(-riskFreeRate*residualTime),
                antitheticVariance));

        // initialize the multi-factor Monte Carlo
        mcModel_ = Handle<MonteCarloModel<MultiAsset_old<
                                          PseudoRandomSequence_old> > > (
            new MonteCarloModel<MultiAsset_old<
                                PseudoRandomSequence_old> > (
            pathGenerator, pathPricer, Statistics(), false));

    }

}

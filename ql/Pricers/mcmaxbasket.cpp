
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

#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class MaxBasketPathPricer : public PathPricer<MultiPath> {
          public:
            MaxBasketPathPricer(const std::vector<double>& underlying,
                                const RelinkableHandle<TermStructure>& 
                                                                discountTS)
            : PathPricer<MultiPath>(discountTS),
              underlying_(underlying) {
                for (Size i=0; i<underlying_.size(); i++) {
                    QL_REQUIRE(underlying_[i]>0.0,
                               "MaxBasketPathPricer: "
                               "underlying less/equal zero not allowed");
                }
            }

            double operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "MaxBasketPathPricer: "
                           "the multi-path must contain "
                           + IntegerFormatter::toString(underlying_.size()) + 
                           " assets");

                double log_variation;
                Size i,j;
                double maxPrice = -QL_MAX_DOUBLE;
                for(j = 0; j < numAssets; j++) {
                    log_variation = 0.0;
                    for(i = 0; i < numSteps; i++)
                        log_variation += multiPath[j][i];
                    maxPrice = QL_MAX(maxPrice,
                                      underlying_[j]*QL_EXP(log_variation));
                }
                return discountTS_->discount(multiPath[0].timeGrid().back())
                    * maxPrice;
            }

          private:
            std::vector<double> underlying_;
        };

    }


    McMaxBasket::McMaxBasket(const std::vector<double>& underlying,
                             const Array& dividendYield, 
                             const Matrix& covariance,
                             Rate riskFreeRate,  double residualTime,
                             long seed) {

        QL_REQUIRE(covariance.rows() == covariance.columns(),
                   "McMaxBasket: covariance matrix not square");
        QL_REQUIRE(covariance.rows() == underlying.size(),
                   "McMaxBasket: underlying size does not match that of"
                   " covariance matrix");
        QL_REQUIRE(covariance.rows() == dividendYield.size(),
                   "McMaxBasket: dividendYield size does not match"
                   " that of covariance matrix");
        QL_REQUIRE(residualTime > 0,
                   "McMaxBasket: residual time must be positive");

        // initialize the path generator
        Array mu(riskFreeRate - dividendYield
                 - 0.5 * covariance.diagonal());

        boost::shared_ptr<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(mu, covariance,
            TimeGrid(residualTime, 1), seed));

        RelinkableHandle<TermStructure> discount(
                  Handle<TermStructure>(
                      new FlatForward(Date::todaysDate(), Date::todaysDate(), 
                                      riskFreeRate)));

        // initialize the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new MaxBasketPathPricer(underlying, discount));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset_old<
                                          PseudoRandomSequence_old> > > (
            new MonteCarloModel<MultiAsset_old<
                                PseudoRandomSequence_old> > (pathGenerator,
            pathPricer, Statistics(), false));

    }

}


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
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class EverestPathPricer : public PathPricer<MultiPath> {
          public:
            EverestPathPricer(const RelinkableHandle<TermStructure>& 
                                                                discountTS)
            : PathPricer<MultiPath>(discountTS) {}

            double operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();

                double log_variation;
                Size i,j;
                double minPrice = QL_MAX_DOUBLE;
                for( j = 0; j < numAssets; j++) {
                    log_variation = 0.0;
                    for( i = 0; i < numSteps; i++)
                        log_variation += multiPath[j][i];
                    minPrice = QL_MIN(minPrice, 
                                      QL_EXP(log_variation));
                }

                return discountTS_->discount(multiPath[0].timeGrid().back())
                    * minPrice;
            }
        };

    }

    McEverest::McEverest(const Array& dividendYield,
                         const Matrix& covariance,
                         Rate riskFreeRate, Time residualTime,
                         long seed) {

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

        RelinkableHandle<TermStructure> discount(
                  Handle<TermStructure>(
                      new FlatForward(Date::todaysDate(), Date::todaysDate(), 
                                      riskFreeRate)));

        // initialize the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new EverestPathPricer(discount));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset_old<
                                          PseudoRandomSequence_old> > > (
            new MonteCarloModel<MultiAsset_old<
                                PseudoRandomSequence_old> > (
            pathGenerator, pathPricer, Statistics(), false));

    }

}

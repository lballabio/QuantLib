
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
                               "underlying less/equal zero not allowed");
                }
            }

            double operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numSteps = multiPath.pathSize();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "the multi-path must contain "
                           + SizeFormatter::toString(underlying_.size()) + 
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


    McMaxBasket::McMaxBasket(
               const std::vector<double>& underlying,
               const std::vector<RelinkableHandle<TermStructure> >& 
                                                             dividendYield,
               const RelinkableHandle<TermStructure>& riskFreeRate,
               const std::vector<RelinkableHandle<BlackVolTermStructure> >& 
                                                             volatilities,
               const Matrix& correlation,
               double residualTime,
               long seed) {

        QL_REQUIRE(correlation.rows() == correlation.columns(),
                   "correlation matrix not square");
        QL_REQUIRE(correlation.rows() == underlying.size(),
                   "underlying size does not match that of"
                   " correlation matrix");
        QL_REQUIRE(correlation.rows() == dividendYield.size(),
                   "dividendYield size does not match"
                   " that of correlation matrix");
        QL_REQUIRE(residualTime > 0,
                   "residual time must be positive");

        // initialize the path generator
        Size n = underlying.size();
        std::vector<boost::shared_ptr<DiffusionProcess> > processes(n);
        for (Size i=0; i<n; i++)
            processes[i] = Handle<DiffusionProcess>(
                    new BlackScholesProcess(riskFreeRate, dividendYield[i],
                                            volatilities[i], underlying[i]));

        TimeGrid grid(residualTime, 1);
        PseudoRandom::rsg_type rsg = 
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        typedef MultiAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                                  new generator(processes, correlation, grid, 
                                                rsg, false));

        // initialize the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new MaxBasketPathPricer(underlying, riskFreeRate));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset<
                                                      PseudoRandom> > > (
            new MonteCarloModel<MultiAsset<PseudoRandom> >(
                             pathGenerator, pathPricer, Statistics(), false));

    }

}

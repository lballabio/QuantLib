
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

    McEverest::McEverest(
                  const std::vector<RelinkableHandle<TermStructure> >& 
                                                             dividendYield,
                  const RelinkableHandle<TermStructure>& riskFreeRate,
                  const std::vector<RelinkableHandle<BlackVolTermStructure> >& 
                                                             volatilities,
                  const Matrix& correlation,
                  Time residualTime,
                  long seed) {

        Size n = correlation.rows();
        QL_REQUIRE(correlation.columns() == n,
                   "McEverest: correlation matrix not square");
        QL_REQUIRE(dividendYield.size() == n,
                   "McEverest: dividendYield size does not match"
                   " that of correlation matrix");
        QL_REQUIRE(residualTime > 0,
                   "McEverest: residualTime must be positive");

        // initialize the path generator
        std::vector<boost::shared_ptr<DiffusionProcess> > processes(n);
        for (Size i=0; i<n; i++)
            processes[i] = Handle<DiffusionProcess>(
                     new BlackScholesProcess(riskFreeRate, dividendYield[i],
                                             volatilities[i], 1.0));

        TimeGrid grid(residualTime, 1);
        PseudoRandom::rsg_type rsg = 
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        boost::shared_ptr<GaussianMultiPathGenerator> pathGenerator(
            new GaussianMultiPathGenerator(processes, correlation, grid, 
                                           rsg, false));

        // initialize the path pricer
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
            new EverestPathPricer(riskFreeRate));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset<
                                                      PseudoRandom> > > (
            new MonteCarloModel<MultiAsset<PseudoRandom> > (
                             pathGenerator, pathPricer, Statistics(), false));

    }

}

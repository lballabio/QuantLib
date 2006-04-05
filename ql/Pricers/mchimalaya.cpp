/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/Pricers/mchimalaya.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Processes/stochasticprocessarray.hpp>

namespace QuantLib {

    namespace {

        class HimalayaPathPricer : public PathPricer<MultiPath> {
          public:
            HimalayaPathPricer(Real strike,
                               DiscountFactor discount)
            : strike_(strike), discount_(discount) {
                QL_REQUIRE(strike>=0.0, "negative strike given");
            }

            Real operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                Size numNodes = multiPath.pathSize();
                QL_REQUIRE(numAssets>0, "no asset given");

                Size i, j;
                std::vector<Real> prices(numAssets);
                for (j=0; j<numAssets; j++)
                    prices[j] = multiPath[j].front();
                std::vector<bool> remainingAssets(numAssets, true);
                Real averagePrice = 0.0;
                Real bestPrice;
                Size removeAsset;
                Size fixings = numNodes-1;
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
                    fixings = numNodes;
                }
                for (i = 1; i < numNodes; i++) {
                    bestPrice = 0.0;
                    // dummy assignement to avoid compiler warning
                    removeAsset=0;
                    for (j = 0; j < numAssets; j++) {
                        if (remainingAssets[j]) {
                            prices[j] = multiPath[j][i];
                            if (prices[j] >= bestPrice) {
                                bestPrice = prices[j];
                                removeAsset = j;
                            }
                        }
                    }
                    remainingAssets[removeAsset] = false;
                    averagePrice += bestPrice;
                }
                averagePrice /= std::min(fixings, numAssets);
                Real optPrice = std::max<Real>(averagePrice - strike_, 0.0);

                return discount_ * optPrice;
            }

          private:
            Real strike_;
            DiscountFactor discount_;
        };

    }

    McHimalaya::McHimalaya(
               const std::vector<Real>& underlying,
               const std::vector<Handle<YieldTermStructure> >& dividendYield,
               const Handle<YieldTermStructure>& riskFreeRate,
               const std::vector<Handle<BlackVolTermStructure> >& volatilities,
               const Matrix& correlation,
               Real strike,
               const std::vector<Time>& times,
               BigNatural seed) {

        Size  n = correlation.rows();
        QL_REQUIRE(correlation.columns() == n,
                   "correlation matrix not square");
        QL_REQUIRE(underlying.size() == n,
                   "underlying size does not match that of"
                   " correlation matrix");
        QL_REQUIRE(dividendYield.size() == n,
                   "dividendYield size does not match"
                   " that of correlation matrix");
        QL_REQUIRE(times.size() >= 1,
                   "you must have at least one time-step");

        // initialize the path generator
        std::vector<boost::shared_ptr<StochasticProcess1D> > processes(n);
        for (Size i=0; i<n; i++) {
            Handle<Quote> u(
                    boost::shared_ptr<Quote>(new SimpleQuote(underlying[i])));
            processes[i] = boost::shared_ptr<StochasticProcess1D>(
                         new GeneralizedBlackScholesProcess(u,
                                                            dividendYield[i],
                                                            riskFreeRate,
                                                            volatilities[i]));
        }
        boost::shared_ptr<StochasticProcess> process(
                           new StochasticProcessArray(processes,correlation));

        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        bool brownianBridge = false;

        typedef MultiVariate<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                           new generator(process, grid, rsg, brownianBridge));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(times.back());
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
                                    new HimalayaPathPricer(strike, discount));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiVariate<
                                                      PseudoRandom> > > (
            new MonteCarloModel<MultiVariate<PseudoRandom> > (
                             pathGenerator, pathPricer, Statistics(), false));

    }

}

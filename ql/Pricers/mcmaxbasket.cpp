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

#include <ql/Pricers/mcmaxbasket.hpp>
#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Processes/stochasticprocessarray.hpp>

namespace QuantLib {

    namespace {

        class MaxBasketPathPricer : public PathPricer<MultiPath> {
          public:
            MaxBasketPathPricer(const std::vector<Real>& underlying,
                                DiscountFactor discount)
            : underlying_(underlying), discount_(discount) {
                for (Size i=0; i<underlying_.size(); i++) {
                    QL_REQUIRE(underlying_[i]>0.0,
                               "underlying less/equal zero not allowed");
                }
            }

            Real operator()(const MultiPath& multiPath) const {
                Size numAssets = multiPath.assetNumber();
                QL_REQUIRE(underlying_.size() == numAssets,
                           "the multi-path must contain "
                           << underlying_.size() << " assets");

                Real maxPrice = QL_MIN_REAL;
                for (Size j = 0; j < numAssets; j++)
                    maxPrice = std::max(maxPrice,multiPath[j].back());

                return discount_ * maxPrice;
            }

          private:
            std::vector<Real> underlying_;
            DiscountFactor discount_;
        };

    }


    McMaxBasket::McMaxBasket(
               const std::vector<Real>& underlying,
               const std::vector<Handle<YieldTermStructure> >& dividendYield,
               const Handle<YieldTermStructure>& riskFreeRate,
               const std::vector<Handle<BlackVolTermStructure> >& volatilities,
               const Matrix& correlation,
               Time residualTime,
               BigNatural seed) {

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
        std::vector<boost::shared_ptr<StochasticProcess1D> > processes(n);
        for (Size i=0; i<n; i++) {
            Handle<Quote> u(
                    boost::shared_ptr<Quote>(new SimpleQuote(underlying[i])));
            processes[i] = boost::shared_ptr<StochasticProcess1D>(
                                    new BlackScholesProcess(u,
                                                            dividendYield[i],
                                                            riskFreeRate,
                                                            volatilities[i]));
        }
        boost::shared_ptr<StochasticProcess> process(
                           new StochasticProcessArray(processes,correlation));

        TimeGrid grid(residualTime, 1);
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(n*(grid.size()-1),seed);

        bool brownianBridge = false;

        typedef MultiAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                           new generator(process, grid, rsg, brownianBridge));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(residualTime);
        boost::shared_ptr<PathPricer<MultiPath> > pathPricer(
                               new MaxBasketPathPricer(underlying, discount));

        // initialize the multi-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<MultiAsset<
                                                      PseudoRandom> > > (
            new MonteCarloModel<MultiAsset<PseudoRandom> >(
                             pathGenerator, pathPricer, Statistics(), false));

    }

}

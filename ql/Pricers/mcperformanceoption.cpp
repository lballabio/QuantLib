
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/Pricers/mcperformanceoption.hpp>
#include <ql/Instruments/payoffs.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class PerformanceOptionPathPricer : public PathPricer<Path> {
          public:
            PerformanceOptionPathPricer(
                                 Option::Type type,
                                 Real underlying,
                                 Real moneyness,
                                 const std::vector<DiscountFactor>& discounts)
            : underlying_(underlying), discounts_(discounts),
              payoff_(type, moneyness) {
                QL_REQUIRE(underlying>0.0,
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(moneyness>0.0,
                           "moneyness less/equal zero not allowed");
            }

            Real operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0,
                           "at least one option is required");
                QL_REQUIRE(n==2,
                           "only one option for the time being");
                QL_REQUIRE(n==discounts_.size(),
                           "discounts/options mismatch");

                std::vector<Real> result(n);
                std::vector<Real> assetValue(n);
                Real log_variation = path[0];
                assetValue[0]  = underlying_ * QL_EXP(log_variation);

                // removing first option
                result[0] = 0.0;
                for (Size i = 1 ; i < n; i++) {
                    log_variation  += path[i];
                    assetValue[i]  = underlying_ * QL_EXP(log_variation);
                    result[i] = discounts_[i] *
                        payoff_(assetValue[i]/assetValue[i-1]);
                }

                return result[1];
            }

          private:
            Real underlying_;
            std::vector<DiscountFactor> discounts_;
            PlainVanillaPayoff payoff_;
        };

    }

    McPerformanceOption::McPerformanceOption(
                              Option::Type type,
                              Real underlying,
                              Real moneyness,
                              const Handle<YieldTermStructure>& dividendYield,
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<BlackVolTermStructure>& volatility,
                              const std::vector<Time>& times,
                              BigNatural seed) {

        std::vector<DiscountFactor> discounts(times.size());
        for (Size i = 0; i<times.size(); i++)
            discounts[i] = riskFreeRate->discount(times[i]);

        // Initialize the path generator
        Handle<Quote> u(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
        boost::shared_ptr<StochasticProcess> diffusion(
                                      new BlackScholesProcess(u,
                                                              dividendYield,
                                                              riskFreeRate,
                                                              volatility));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        typedef SingleAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                                  new generator(diffusion, grid, rsg, false));

        // Initialize the pricer on the single Path
        boost::shared_ptr<PathPricer<Path> > performancePathPricer(
            new PerformanceOptionPathPricer(type,
            underlying, moneyness, discounts));

        // Initialize the one-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                          PseudoRandom> > > (
            new MonteCarloModel<SingleAsset<PseudoRandom> > (
                pathGenerator, performancePathPricer,
                Statistics(), false));
    }

}


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
                                 double underlying,
                                 double moneyness,
                                 const std::vector<DiscountFactor>& discounts)
            : underlying_(underlying), discounts_(discounts), 
              payoff_(type, moneyness) {
                QL_REQUIRE(underlying>0.0,
                           "PerformanceOptionPathPricer: "
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(moneyness>0.0,
                           "PerformanceOptionPathPricer: "
                           "moneyness less/equal zero not allowed");
            }

            double operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0,
                           "PerformanceOptionPathPricer: "
                           "at least one option is required");
                QL_REQUIRE(n==2,
                           "PerformanceOptionPathPricer: "
                           "only one option for the time being");
                QL_REQUIRE(n==discounts_.size(),
                           "PerformanceOptionPathPricer: "
                           "discounts/options mismatch");

                std::vector<double> result(n);
                std::vector<double> assetValue(n);
                double log_variation = path[0];
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
            double underlying_;
            std::vector<DiscountFactor> discounts_;
            PlainVanillaPayoff payoff_;
        };

    }

    McPerformanceOption::McPerformanceOption(
                    Option::Type type,
                    double underlying,
                    double moneyness,
                    const RelinkableHandle<TermStructure>& dividendYield,
                    const RelinkableHandle<TermStructure>& riskFreeRate,
                    const RelinkableHandle<BlackVolTermStructure>& volatility,
                    const std::vector<Time>& times,
                    long seed) {

        std::vector<double> discounts(times.size());
        for (Size i = 0; i<times.size(); i++)
            discounts[i] = riskFreeRate->discount(times[i]);

        // Initialize the path generator
        boost::shared_ptr<DiffusionProcess> diffusion(
                          new BlackScholesProcess(riskFreeRate, dividendYield,
                                                  volatility, underlying));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        boost::shared_ptr<GaussianPathGenerator> pathGenerator(
                      new GaussianPathGenerator(diffusion, grid, rsg, false));

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

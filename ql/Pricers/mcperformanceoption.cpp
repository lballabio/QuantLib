
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
                       Option::Type type, double underlying, double moneyness,
                       const std::vector<Spread>& dividendYield,
                       const std::vector<Rate>& riskFreeRate,
                       const std::vector<Time>& times,
                       const std::vector<double>& volatility,
                       long seed) {

        Size dimension = times.size();
        QL_REQUIRE(dividendYield.size()==dimension,
                   "McPerformanceOption: dividendYield vector of wrong size");
        QL_REQUIRE(riskFreeRate.size()==dimension,
                   "McPerformanceOption: riskFreeRate vector of wrong size");
        QL_REQUIRE(volatility.size()==dimension,
                   "McPerformanceOption: volatility vector of wrong size");

        // Initialize the path generator
        std::vector<double> mu(dimension);
        std::vector<double> diffusion(dimension);
        std::vector<double> discounts(dimension);
        for (Size i = 0; i<dimension; i++) {
            mu[i]= riskFreeRate[i] - dividendYield[i] -
                0.5 * volatility[i] * volatility[i];
            diffusion[i]= volatility[i] * volatility[i];
            if (i==0)
                discounts[i] = QL_EXP(-riskFreeRate[i]*times[i]);
            else
                discounts[i] = discounts[i-1]*
                    QL_EXP(-riskFreeRate[i]*(times[i]-times[i-1]));
        }


        boost::shared_ptr<GaussianPathGenerator_old> pathGenerator(
            new GaussianPathGenerator_old(mu, diffusion,
                TimeGrid(times.begin(), times.end()),
                seed));

        // Initialize the pricer on the single Path
        boost::shared_ptr<PathPricer<Path> > performancePathPricer(
            new PerformanceOptionPathPricer(type,
            underlying, moneyness, discounts));

        // Initialize the one-factor Monte Carlo
        mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                          PseudoRandom_old> > > (
            new MonteCarloModel<SingleAsset_old<
                                PseudoRandom_old> > (
                pathGenerator, performancePathPricer,
                Statistics(), false));
    }

}


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

#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticAPOPathPricer_old : public PathPricer_old<Path> {
          public:
            ArithmeticAPOPathPricer_old(Option::Type type,
                                        double underlying,
                                        double strike,
                                        DiscountFactor discount,
                                        bool useAntitheticVariance)
            : PathPricer_old<Path>(discount, useAntitheticVariance),
              underlying_(underlying), payoff_(type, strike) {
                QL_REQUIRE(underlying>0.0,
                           "ArithmeticAPOPathPricer_old: "
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "ArithmeticAPOPathPricer_old: "
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const  {

                Size n = path.size();
                QL_REQUIRE(n>0,
                           "ArithmeticAPOPathPricer_old: "
                           "the path cannot be empty");

                double price1 = underlying_;
                double averagePrice1 = 0.0;
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    averagePrice1 = price1;
                    fixings = n+1;
                }
                Size i;
                for (i=0; i<n; i++) {
                    price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                    averagePrice1 += price1;
                }
                averagePrice1 = averagePrice1/fixings;

                if (useAntitheticVariance_) {
                    double price2 = underlying_;
                    double averagePrice2 = 0.0;

                    if (path.timeGrid().mandatoryTimes()[0]==0.0)
                        averagePrice2 = price2;
                    for (i=0; i<n; i++) {
                        price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                        averagePrice2 += price2;
                    }
                    averagePrice2 = averagePrice2/fixings;
                    return discount_/2.0 *
                        (payoff_(averagePrice1)+payoff_(averagePrice2));
                } else
                    return discount_ * payoff_(averagePrice1);
            }

          private:
            double underlying_;
            PlainVanillaPayoff payoff_;
        };

        class GeometricAPOPathPricer_old : public PathPricer_old<Path> {
          public:
            GeometricAPOPathPricer_old(Option::Type type,
                                       double underlying,
                                       double strike,
                                       DiscountFactor discount,
                                       bool useAntitheticVariance)
            : PathPricer_old<Path>(discount, useAntitheticVariance),
              underlying_(underlying), payoff_(type, strike) {
                QL_REQUIRE(underlying>0.0,
                           "GeometricAPOPathPricer_old: "
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "GeometricAPOPathPricer_old: "
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0,
                           "GeometricAPOPathPricer_old: "
                           "the path cannot be empty");

                double geoLogDrift = 0.0, geoLogDiffusion = 0.0;
                Size i;
                for (i=0; i<n; i++) {
                    geoLogDrift += (n-i)*path.drift()[i];
                    geoLogDiffusion += (n-i)*path.diffusion()[i];
                }
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    fixings = n+1;
                }
                double averagePrice1 = underlying_*
                    QL_EXP((geoLogDrift+geoLogDiffusion)/fixings);

                if (useAntitheticVariance_) {
                    double averagePrice2 = underlying_*
                        QL_EXP((geoLogDrift-geoLogDiffusion)/fixings);
                    return discount_ * 0.5 *
                        (payoff_(averagePrice1) +payoff_(averagePrice2));
                } else {
                    return discount_* payoff_(averagePrice1);
                }
            }

          private:
            double underlying_;
            PlainVanillaPayoff payoff_;
        };

    }


    McDiscreteArithmeticAPO::McDiscreteArithmeticAPO(Option::Type type,
        double underlying, double strike,
        Spread dividendYield, Rate riskFreeRate,
        const std::vector<Time>& times, double volatility,
        bool antitheticVariance, bool controlVariate, long seed) {

        QL_REQUIRE(times.size() >= 2,
                   "McDiscreteArithmeticAPO: "
                   "you must have at least 2 time-steps");

        // initialize the path generator
        double mu = riskFreeRate - dividendYield
                                 - 0.5 * volatility * volatility;

        boost::shared_ptr<GaussianPathGenerator_old> pathGenerator(
            new GaussianPathGenerator_old(mu, volatility*volatility,
                TimeGrid(times.begin(), times.end()),
                seed));


        // initialize the Path Pricer
        boost::shared_ptr<PathPricer_old<Path> > spPricer(
            new ArithmeticAPOPathPricer_old(type, underlying, strike,
                QL_EXP(-riskFreeRate*times.back()), antitheticVariance));


        if (controlVariate) {
            boost::shared_ptr<PathPricer_old<Path> > controlVariateSpPricer(
                new GeometricAPOPathPricer_old(type, underlying, strike,
                    QL_EXP(-riskFreeRate*times.back()), antitheticVariance));

            double controlVariatePrice = DiscreteGeometricAPO(type,
                underlying, strike, dividendYield, riskFreeRate,
                times, volatility).value();

            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > >(
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
